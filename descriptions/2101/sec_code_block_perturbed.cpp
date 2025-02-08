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