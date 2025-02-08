GDALDataset *FITDataset::Open( GDALOpenInfo * openInfo )
{
/* -------------------------------------------------------------------- */
/*      First we check to see if the file has the expected header       */
/*      bytes.                                                          */
/* -------------------------------------------------------------------- */

    if( openInfo->nHeaderBytes < 5 )
        return NULL;

    if( !STARTS_WITH_CI((const char *) openInfo->pabyHeader, "IT01") &&
        !STARTS_WITH_CI((const char *) openInfo->pabyHeader, "IT02") )
        return NULL;

    if( openInfo->eAccess == GA_Update )
    {
        CPLError( CE_Failure, CPLE_NotSupported,
                  "The FIT driver does not support update access to existing"
                  " files.\n" );
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Create a corresponding GDALDataset.                             */
/* -------------------------------------------------------------------- */
    FITDataset *poDS = new FITDataset();
    DeleteGuard<FITDataset> guard( poDS );

    // Re-open file for large file (64bit) access.
    if ( openInfo->eAccess == GA_ReadOnly )
        poDS->fp = VSIFOpenL( openInfo->pszFilename, "rb" );
    else
        poDS->fp = VSIFOpenL( openInfo->pszFilename, "r+b" );

    if ( !poDS->fp )
    {
        CPLError( CE_Failure, CPLE_OpenFailed,
                  "Failed to re-open %s with FIT driver.\n",
                  openInfo->pszFilename );
        return NULL;
    }
    poDS->eAccess = openInfo->eAccess;

    poDS->info = new FITinfo;
    FITinfo *info = poDS->info;

/* -------------------------------------------------------------------- */
/*      Read other header values.                                       */
/* -------------------------------------------------------------------- */
    FIThead02 *head = (FIThead02 *) openInfo->pabyHeader;

    // extract the image attributes from the file header
    if (STARTS_WITH_CI((const char *) &head->version, "02")) {
        // incomplete header
        if( openInfo->nHeaderBytes < (signed) sizeof(FIThead02) )
            return NULL;

        CPLDebug("FIT", "Loading file with header version 02");

        gst_swapb(head->minValue);
        info->minValue = head->minValue;
        gst_swapb(head->maxValue);
        info->maxValue = head->maxValue;
        gst_swapb(head->dataOffset);
        info->dataOffset = head->dataOffset;

        info->userOffset = sizeof(FIThead02);
    }
    else if (STARTS_WITH_CI((const char *) &head->version, "01")) {
        // incomplete header
        if( openInfo->nHeaderBytes < (signed) sizeof(FIThead01) )
            return NULL;

        CPLDebug("FIT", "Loading file with header version 01");

        // map old style header into new header structure
        FIThead01* head01 = (FIThead01*)head;
        gst_swapb(head->dataOffset);
        info->dataOffset = head01->dataOffset;

        info->userOffset = sizeof(FIThead01);
    }
    else {
        // unrecognized header version
        CPLError( CE_Failure, CPLE_NotSupported,
                  "FIT - unsupported header version %.2s\n",
                  (const char*) &head->version);
        return NULL;
    }

    CPLDebug("FIT", "userOffset %i, dataOffset %i",
             info->userOffset, info->dataOffset);

    info->magic = head->magic;
    info->version = head->version;

    gst_swapb(head->xSize);
    info->xSize = head->xSize;
    gst_swapb(head->ySize);
    info->ySize = head->ySize;
    gst_swapb(head->zSize);
    info->zSize = head->zSize;
    gst_swapb(head->cSize);
    info->cSize = head->cSize;
    gst_swapb(head->dtype);
    info->dtype = head->dtype;
    gst_swapb(head->order);
    info->order = head->order;
    gst_swapb(head->space);
    info->space = head->space;
    gst_swapb(head->cm);
    info->cm = head->cm;
    gst_swapb(head->xPageSize);
    info->xPageSize = head->xPageSize;
    gst_swapb(head->yPageSize);
    info->yPageSize = head->yPageSize;
    gst_swapb(head->zPageSize);
    info->zPageSize = head->zPageSize;
    gst_swapb(head->cPageSize);
    info->cPageSize = head->cPageSize;

    CPLDebug("FIT", "size %i %i %i %i, pageSize %i %i %i %i",
             info->xSize, info->ySize, info->zSize, info->cSize,
             info->xPageSize, info->yPageSize, info->zPageSize,
             info->cPageSize);

    CPLDebug("FIT", "dtype %i order %i space %i cm %i",
             info->dtype, info->order, info->space, info->cm);

    /**************************/

    poDS->nRasterXSize = head->xSize;
    poDS->nRasterYSize = head->ySize;

    if (!GDALCheckDatasetDimensions(poDS->nRasterXSize, poDS->nRasterYSize) ||
        !GDALCheckBandCount(head->cSize, FALSE) ||
        head->xPageSize == 0 ||
        head->yPageSize == 0)
    {
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Verify all "unused" header values.                              */
/* -------------------------------------------------------------------- */

    if( info->zSize != 1 )
    {
        CPLError( CE_Failure, CPLE_NotSupported,
                  "FIT driver - unsupported zSize %i\n", info->zSize);
        return NULL;
    }

    if( info->order != 1 ) // interleaved - RGBRGB
    {
        CPLError( CE_Failure, CPLE_NotSupported,
                  "FIT driver - unsupported order %i\n", info->order);
        return NULL;
    }

    if( info->zPageSize != 1 )
    {
        CPLError( CE_Failure, CPLE_NotSupported,
                  "FIT driver - unsupported zPageSize %i\n", info->zPageSize);
        return NULL;
    }

    if( info->cPageSize != info->cSize )
    {
        CPLError( CE_Failure, CPLE_NotSupported,
                  "FIT driver - unsupported cPageSize %i (!= %i)\n",
                  info->cPageSize, info->cSize);
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Create band information objects.                                */
/* -------------------------------------------------------------------- */
    for( int i = 0; i < (int)head->cSize; i++ )
    {
        FITRasterBand* poBand = new FITRasterBand( poDS, i+1, (int)head->cSize );
        poDS->SetBand( i+1,  poBand);
        if( poBand->tmpImage == NULL )
            return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Initialize any PAM information.                                 */
/* -------------------------------------------------------------------- */
    poDS->SetDescription( openInfo->pszFilename );
    poDS->TryLoadXML();

/* -------------------------------------------------------------------- */
/*      Check for external overviews.                                   */
/* -------------------------------------------------------------------- */
    poDS->oOvManager.Initialize( poDS, openInfo->pszFilename, openInfo->GetSiblingFiles() );

    return guard.take();
}