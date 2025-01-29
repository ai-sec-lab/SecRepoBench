if (!GDALCheckDatasetDimensions(poDS->nRasterXSize, poDS->nRasterYSize) ||
        !GDALCheckBandCount(head->cSize, FALSE) ||
        head->xPageSize == 0 ||
        head->yPageSize == 0)
    {
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Check if 64 bit seek is needed.                                 */
/* -------------------------------------------------------------------- */
    uint64 bytesPerComponent =
        GDALGetDataTypeSize(fitDataType(poDS->info->dtype)) / 8;
    uint64 bytesPerPixel = head->cSize * bytesPerComponent;
    uint64 recordSize = bytesPerPixel * head->xPageSize *
        head->yPageSize;
    uint64 numXBlocks =
        (uint64) ceil((double) head->xSize / head->xPageSize);
    uint64 numYBlocks =
        (uint64) ceil((double) head->ySize / head->yPageSize);

    uint64 maxseek = recordSize * numXBlocks * numYBlocks;

//     CPLDebug("FIT", "(sizeof %i) max seek %llx ==> %llx\n", sizeof(uint64),
//              maxseek, maxseek >> 31);
    if (maxseek >> 31) // signed long
#ifdef VSI_LARGE_API_SUPPORTED
        CPLDebug("FIT", "Using 64 bit version of fseek");
#else
        CPLError(CE_Fatal, CPLE_NotSupported,
                 "FIT - need 64 bit version of fseek");
#endif

/* -------------------------------------------------------------------- */
/*      Verify all "unused" header values.                              */
/* -------------------------------------------------------------------- */

    if( info->zSize != 1 )
    {
        CPLError( CE_Failure, CPLE_NotSupported,
                  "FIT driver - unsupported zSize %i\n", info->zSize);
        return NULL;
    }