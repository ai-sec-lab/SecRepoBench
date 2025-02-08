static CPLErr
GDALResampleChunk32R_Gauss( double dfXRatioDstToSrc, double dfYRatioDstToSrc,
                            double /* dfSrcXDelta */,
                            double /* dfSrcYDelta */,
                            GDALDataType /* eWrkDataType */,
                            void * pChunk,
                            GByte * pabyChunkNodataMask,
                            int nChunkXOff, int nChunkXSize,
                            int nChunkYOff, int nChunkHeight,
                            int nDstXOff, int nDstXOff2,
                            int nDstYOff, int nDstYOff2,
                            GDALRasterBand * poOverview,
                            const char * /* pszResampling */,
                            int bHasNoData, float fNoDataValue,
                            GDALColorTable* poColorTable,
                            GDALDataType /* eSrcDataType */,
                            bool /* bPropagateNoData */ )

{
    float * pafChunk = static_cast<float *>( pChunk );

/* -------------------------------------------------------------------- */
/*      Create the filter kernel and allocate scanline buffer.          */
/* -------------------------------------------------------------------- */
    int nGaussMatrixDim = 3;
    const int *panGaussMatrix;
    static const int anGaussMatrix3x3[] ={
        1, 2, 1,
        2, 4, 2,
        1, 2, 1
    };
    static const int anGaussMatrix5x5[] = {
        1, 4, 6, 4, 1,
        4, 16, 24, 16, 4,
        6, 24, 36, 24, 6,
        4, 16, 24, 16, 4,
        1, 4, 6, 4, 1};
    static const int anGaussMatrix7x7[] = {
        1, 6, 15, 20, 15, 6, 1,
        6, 36, 90, 120, 90, 36, 6,
        15, 90, 225, 300, 225, 90, 15,
        20, 120, 300, 400, 300, 120, 20,
        15, 90, 225, 300, 225, 90, 15,
        6, 36, 90, 120, 90, 36, 6,
        1, 6, 15, 20, 15, 6, 1};

    const int nOXSize = poOverview->GetXSize();
    const int nOYSize = poOverview->GetYSize();
    const int nResYFactor = static_cast<int>(0.5 + dfYRatioDstToSrc);

    // matrix for gauss filter
    if(nResYFactor <= 2 )
    {
        panGaussMatrix = anGaussMatrix3x3;
        nGaussMatrixDim=3;
    }
    else if( nResYFactor <= 4 )
    {
        panGaussMatrix = anGaussMatrix5x5;
        nGaussMatrixDim=5;
    }
    else
    {
        panGaussMatrix = anGaussMatrix7x7;
        nGaussMatrixDim=7;
    }

// <MASK>

    return eErr;
}