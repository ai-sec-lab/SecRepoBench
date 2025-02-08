GDALDatasetH CPL_STDCALL GDALOpenEx( const char *pszFilename,
                                     unsigned int nOpenFlags,
                                     const char *const *papszAllowedDrivers,
                                     const char *const *papszOpenOptions,
                                     const char *const *papszSiblingFiles )
{
    VALIDATE_POINTER1(pszFilename, "GDALOpen", NULL);

/* -------------------------------------------------------------------- */
/*      In case of shared dataset, first scan the existing list to see  */
/*      if it could already contain the requested dataset.              */
/* -------------------------------------------------------------------- */
    if( nOpenFlags & GDAL_OF_SHARED )
    {
        if( nOpenFlags & GDAL_OF_INTERNAL )
        {
            CPLError(CE_Failure, CPLE_IllegalArg,
                     "GDAL_OF_SHARED and GDAL_OF_INTERNAL are exclusive");
            return NULL;
        }

        CPLMutexHolderD(&hDLMutex);

        if (phSharedDatasetSet != NULL)
        {
            const GIntBig nThisPID = GDALGetResponsiblePIDForCurrentThread();
            SharedDatasetCtxt sharedDatasetContext;

            sharedDatasetContext.nPID = nThisPID;
            sharedDatasetContext.pszDescription = const_cast<char *>(pszFilename);
            sharedDatasetContext.eAccess =
                (nOpenFlags & GDAL_OF_UPDATE) ? GA_Update : GA_ReadOnly;
            SharedDatasetCtxt *psStruct = static_cast<SharedDatasetCtxt *>(
                CPLHashSetLookup(phSharedDatasetSet, &sharedDatasetContext));
            if (psStruct == NULL && (nOpenFlags & GDAL_OF_UPDATE) == 0)
            {
                sharedDatasetContext.eAccess = GA_Update;
                psStruct = static_cast<SharedDatasetCtxt *>(
                    CPLHashSetLookup(phSharedDatasetSet, &sharedDatasetContext));
            }
            if (psStruct)
            {
                psStruct->poDS->Reference();
                return psStruct->poDS;
            }
        }
    }

    // If no driver kind is specified, assume all are to be probed.
    if( (nOpenFlags & GDAL_OF_KIND_MASK) == 0 )
        nOpenFlags |= GDAL_OF_KIND_MASK;

    GDALDriverManager *poDM = GetGDALDriverManager();
    // CPLLocaleC  oLocaleForcer;

    CPLErrorReset();
    VSIErrorReset();
    CPLAssert(NULL != poDM);

    // Build GDALOpenInfo just now to avoid useless file stat'ing if a
    // shared dataset was asked before.
    GDALOpenInfo oOpenInfo(pszFilename, nOpenFlags,
                           const_cast<char **>(papszSiblingFiles));
    oOpenInfo.papszAllowedDrivers = papszAllowedDrivers;

    // Prevent infinite recursion.
    {
        int *pnRecCount =
            static_cast<int *>(CPLGetTLS(CTLS_GDALDATASET_REC_PROTECT_MAP));
        if( pnRecCount == NULL )
        {
            pnRecCount = static_cast<int *>(CPLMalloc(sizeof(int)));
            *pnRecCount = 0;
            CPLSetTLS(CTLS_GDALDATASET_REC_PROTECT_MAP, pnRecCount, TRUE);
        }
        if( *pnRecCount == 100 )
        {
            CPLError(CE_Failure, CPLE_AppDefined,
                     "GDALOpen() called with too many recursion levels");
            return NULL;
        }
        (*pnRecCount)++;
    }

    // Remove leading @ if present.
    char **papszOpenOptionsCleaned =
        CSLDuplicate(const_cast<char **>(papszOpenOptions));
    for(char **papszIter = papszOpenOptionsCleaned; papszIter && *papszIter;
        ++papszIter)
    {
        char *pszOption = *papszIter;
        if( pszOption[0] == '@' )
            memmove(pszOption, pszOption + 1, strlen(pszOption + 1) + 1);
    }

    oOpenInfo.papszOpenOptions = papszOpenOptionsCleaned;

    for( int iDriver = -1; iDriver < poDM->GetDriverCount(); ++iDriver )
    {
        GDALDriver *poDriver = NULL;

        if( iDriver < 0 )
        {
            poDriver = GDALGetAPIPROXYDriver();
        }
        else
        {
            poDriver = poDM->GetDriver(iDriver);
            if (papszAllowedDrivers != NULL &&
                CSLFindString(papszAllowedDrivers,
                              GDALGetDriverShortName(poDriver)) == -1)
                continue;
        }

        if( (nOpenFlags & GDAL_OF_RASTER) != 0 &&
            (nOpenFlags & GDAL_OF_VECTOR) == 0 &&
            poDriver->GetMetadataItem(GDAL_DCAP_RASTER) == NULL )
            continue;
        if( (nOpenFlags & GDAL_OF_VECTOR) != 0 &&
            (nOpenFlags & GDAL_OF_RASTER) == 0 &&
            poDriver->GetMetadataItem(GDAL_DCAP_VECTOR) == NULL )
            continue;

        // Remove general OVERVIEW_LEVEL open options from list before passing
        // it to the driver, if it isn't a driver specific option already.
        char **papszTmpOpenOptions = NULL;
        char **papszTmpOpenOptionsToValidate = NULL;
        char **papszOptionsToValidate = const_cast<char **>(papszOpenOptions);
        if( CSLFetchNameValue(papszOpenOptionsCleaned, "OVERVIEW_LEVEL") !=
               NULL &&
            (poDriver->GetMetadataItem(GDAL_DMD_OPENOPTIONLIST) == NULL ||
             CPLString(poDriver->GetMetadataItem(GDAL_DMD_OPENOPTIONLIST))
                    .ifind("OVERVIEW_LEVEL") == std::string::npos) )
        {
            papszTmpOpenOptions = CSLDuplicate(papszOpenOptionsCleaned);
            papszTmpOpenOptions =
                CSLSetNameValue(papszTmpOpenOptions, "OVERVIEW_LEVEL", NULL);
            oOpenInfo.papszOpenOptions = papszTmpOpenOptions;

            papszOptionsToValidate = CSLDuplicate(papszOptionsToValidate);
            papszOptionsToValidate =
                CSLSetNameValue(papszOptionsToValidate, "OVERVIEW_LEVEL", NULL);
            papszTmpOpenOptionsToValidate = papszOptionsToValidate;
        }

        const bool bIdentifyRes =
            poDriver->pfnIdentify && poDriver->pfnIdentify(&oOpenInfo) > 0;
        if( bIdentifyRes )
        {
            GDALValidateOpenOptions(poDriver, papszOptionsToValidate);
        }

// <MASK>
    }

    CSLDestroy(papszOpenOptionsCleaned);

    if( nOpenFlags & GDAL_OF_VERBOSE_ERROR )
    {
        // Check to see if there was a filesystem error, and report it if so.
        // If not, return a more generic error.
        if(!VSIToCPLError(CE_Failure, CPLE_OpenFailed))
        {
            if( oOpenInfo.bStatOK )
            {
                CPLError(CE_Failure, CPLE_OpenFailed,
                         "`%s' not recognized as a supported file format.",
                         pszFilename);
            }
            else
            {
                // If Stat failed and no VSI error was set, assume it is because
                // the file did not exist on the filesystem.
                CPLError(CE_Failure, CPLE_OpenFailed,
                         "`%s' does not exist in the file system, "
                         "and is not recognized as a supported dataset name.",
                         pszFilename);
            }
        }
    }

    int *pnRecCount =
        static_cast<int *>(CPLGetTLS(CTLS_GDALDATASET_REC_PROTECT_MAP));
    if( pnRecCount )
        (*pnRecCount)--;

    return NULL;
}