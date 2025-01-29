#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
        const bool bFpAvailableBefore = oOpenInfo.fpL != NULL;
#endif

        GDALDataset *poDS = NULL;
        if ( poDriver->pfnOpen != NULL )
        {
            poDS = poDriver->pfnOpen(&oOpenInfo);
            // If we couldn't determine for sure with Identify() (it returned
            // -1), but Open() managed to open the file, post validate options.
            if( poDS != NULL && poDriver->pfnIdentify && !bIdentifyRes )
                GDALValidateOpenOptions(poDriver, papszOptionsToValidate);
        }
        else if( poDriver->pfnOpenWithDriverArg != NULL )
        {
            poDS = poDriver->pfnOpenWithDriverArg(poDriver, &oOpenInfo);
        }
        else
        {
            CSLDestroy(papszTmpOpenOptions);
            CSLDestroy(papszTmpOpenOptionsToValidate);
            oOpenInfo.papszOpenOptions = papszOpenOptionsCleaned;
            continue;
        }

        CSLDestroy(papszTmpOpenOptions);
        CSLDestroy(papszTmpOpenOptionsToValidate);
        oOpenInfo.papszOpenOptions = papszOpenOptionsCleaned;

        if( poDS != NULL )
        {
            poDS->nOpenFlags = nOpenFlags;

            if( strlen(poDS->GetDescription()) == 0 )
                poDS->SetDescription(pszFilename);

            if( poDS->poDriver == NULL )
                poDS->poDriver = poDriver;

            if( poDS->papszOpenOptions == NULL )
            {
                poDS->papszOpenOptions = papszOpenOptionsCleaned;
                papszOpenOptionsCleaned = NULL;
            }

            if( !(nOpenFlags & GDAL_OF_INTERNAL) )
            {
                if( CPLGetPID() != GDALGetResponsiblePIDForCurrentThread() )
                    CPLDebug("GDAL",
                             "GDALOpen(%s, this=%p) succeeds as "
                             "%s (pid=%d, responsiblePID=%d).",
                             pszFilename, poDS, poDriver->GetDescription(),
                             static_cast<int>(CPLGetPID()),
                             static_cast<int>(
                                 GDALGetResponsiblePIDForCurrentThread()));
                else
                    CPLDebug("GDAL", "GDALOpen(%s, this=%p) succeeds as %s.",
                             pszFilename, poDS, poDriver->GetDescription());

                poDS->AddToDatasetOpenList();
            }

            int *pnRecCount =
                static_cast<int *>(CPLGetTLS(CTLS_GDALDATASET_REC_PROTECT_MAP));
            if( pnRecCount )
                (*pnRecCount)--;

            if( nOpenFlags & GDAL_OF_SHARED )
            {
                if (strcmp(pszFilename, poDS->GetDescription()) != 0)
                {
                    CPLError(CE_Warning, CPLE_NotSupported,
                             "A dataset opened by GDALOpenShared should have "
                             "the same filename (%s) "
                             "and description (%s)",
                             pszFilename, poDS->GetDescription());
                }
                else
                {
                    poDS->MarkAsShared();
                }
            }

            // Deal with generic OVERVIEW_LEVEL open option, unless it is
            // driver specific.
            if( CSLFetchNameValue(papszOpenOptions, "OVERVIEW_LEVEL") != NULL &&
                (poDriver->GetMetadataItem(GDAL_DMD_OPENOPTIONLIST) == NULL ||
                CPLString(poDriver->GetMetadataItem(GDAL_DMD_OPENOPTIONLIST))
                        .ifind("OVERVIEW_LEVEL") == std::string::npos) )
            {
                CPLString osVal(
                    CSLFetchNameValue(papszOpenOptions, "OVERVIEW_LEVEL"));
                const int nOvrLevel = atoi(osVal);
                const bool bThisLevelOnly =
                    osVal.ifind("only") != std::string::npos;
                GDALDataset *poOvrDS = GDALCreateOverviewDataset(
                    poDS, nOvrLevel, bThisLevelOnly);
                poDS->ReleaseRef();
                poDS = poOvrDS;
                if( poDS == NULL )
                {
                    if( nOpenFlags & GDAL_OF_VERBOSE_ERROR )
                    {
                        CPLError(CE_Failure, CPLE_OpenFailed,
                                 "Cannot open overview level %d of %s",
                                 nOvrLevel, pszFilename);
                    }
                }
            }
            VSIErrorReset();

            CSLDestroy(papszOpenOptionsCleaned);
            return poDS;
        }

#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
        if( bFpAvailableBefore && oOpenInfo.fpL == NULL )
        {
            // In case the file descriptor was "consumed" by a driver
            // that ultimately failed, re-open it for next drivers.
            oOpenInfo.fpL = VSIFOpenL(
                pszFilename,
                (oOpenInfo.eAccess == GA_Update) ? "r+b" : "rb");
        }
#else
        if( CPLGetLastErrorNo() != 0 && CPLGetLastErrorType() > CE_Warning)
        {
            int *pnRecCount =
                static_cast<int *>(CPLGetTLS(CTLS_GDALDATASET_REC_PROTECT_MAP));
            if( pnRecCount )
                (*pnRecCount)--;

            CSLDestroy(papszOpenOptionsCleaned);
            return NULL;
        }
#endif