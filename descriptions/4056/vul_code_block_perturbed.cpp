    float *pafDstScanline = static_cast<float *>(
        VSI_MALLOC_VERBOSE((nDstXOff2 - nDstXOff) * sizeof(float)) );
    if( pafDstScanline == NULL )
    {
        return CE_Failure;
    }

    if( !bHasNoData )
        fNoDataValue = 0.0f;

    int nEntryCount = 0;
    GDALColorEntry* aEntries = NULL;
    int nTransparentIdx = -1;
    if( poColorTable &&
        !ReadColorTableAsArray(poColorTable, nEntryCount, aEntries,
                               nTransparentIdx) )
    {
        VSIFree(pafDstScanline);
        return CE_Failure;
    }

    // Force c4 of nodata entry to 0 so that GDALFindBestEntry() identifies
    // it as nodata value.
    if( bHasNoData && fNoDataValue >= 0.0f && fNoDataValue < nEntryCount )
    {
        if( aEntries == NULL )
        {
            CPLError(CE_Failure, CPLE_ObjectNull, "No aEntries");
            VSIFree(pafDstScanline);
            return CE_Failure;
        }
        aEntries[static_cast<int>(fNoDataValue)].c4 = 0;
    }
    // Or if we have no explicit nodata, but a color table entry that is
    // transparent, consider it as the nodata value.
    else if( !bHasNoData && nTransparentIdx >= 0 )
    {
        fNoDataValue = static_cast<float>(nTransparentIdx);
    }

    const int nChunkRightXOff = nChunkXOff + nChunkXSize;
    const int nChunkBottomYOff = nChunkYOff + nChunkHeight;

/* ==================================================================== */
/*      Loop over destination scanlines.                                */
/* ==================================================================== */
    CPLErr eErr = CE_None;
    for( int iDstLine = nDstYOff;
         iDstLine < nDstYOff2 && eErr == CE_None;
         ++iDstLine )
    {
        int nSrcYOff = static_cast<int>(0.5 + iDstLine * dfYRatioDstToSrc);
        int nSrcYOff2 =
            static_cast<int>(0.5 + (iDstLine+1) * dfYRatioDstToSrc) + 1;

        if( nSrcYOff < nChunkYOff )
        {
            nSrcYOff = nChunkYOff;
            nSrcYOff2++;
        }

        const int iSizeY = nSrcYOff2 - nSrcYOff;
        nSrcYOff = nSrcYOff + iSizeY/2 - nGaussMatrixDim/2;
        nSrcYOff2 = nSrcYOff + nGaussMatrixDim;
        int nYShiftGaussMatrix = 0;
        if(nSrcYOff < 0)
        {
            nYShiftGaussMatrix = -nSrcYOff;
            nSrcYOff = 0;
        }

        if( nSrcYOff2 > nChunkBottomYOff ||
            (dfYRatioDstToSrc > 1 && iDstLine == nOYSize-1) )
            nSrcYOff2 = nChunkBottomYOff;

        const float * const pafSrcScanline =
            pafChunk + ((nSrcYOff-nChunkYOff) * nChunkXSize);
        GByte *pabySrcScanlineNodataMask = NULL;
        if( pabyChunkNodataMask != NULL )
            pabySrcScanlineNodataMask =
                pabyChunkNodataMask + ((nSrcYOff-nChunkYOff) * nChunkXSize);

/* -------------------------------------------------------------------- */
/*      Loop over destination pixels                                    */
/* -------------------------------------------------------------------- */
        for( int iDstPixel = nDstXOff; iDstPixel < nDstXOff2; ++iDstPixel )
        {
            int nSrcXOff = static_cast<int>(0.5 + iDstPixel * dfXRatioDstToSrc);
            int nSrcXOff2 =
                static_cast<int>(0.5 + (iDstPixel+1) * dfXRatioDstToSrc) + 1;

            const int iSizeX = nSrcXOff2 - nSrcXOff;
            nSrcXOff = nSrcXOff + iSizeX/2 - nGaussMatrixDim/2;
            nSrcXOff2 = nSrcXOff + nGaussMatrixDim;
            int nXShiftGaussMatrix = 0;
            if(nSrcXOff < 0)
            {
                nXShiftGaussMatrix = -nSrcXOff;
                nSrcXOff = 0;
            }

            if( nSrcXOff2 > nChunkRightXOff ||
                (dfXRatioDstToSrc > 1 && iDstPixel == nOXSize-1) )
                nSrcXOff2 = nChunkRightXOff;

            if( poColorTable == NULL )
            {
                double dfTotal = 0.0;
                int nCount = 0;
                const int *panLineWeight = panGaussMatrix +
                    nYShiftGaussMatrix * nGaussMatrixDim + nXShiftGaussMatrix;

                for( int j=0, iY = nSrcYOff;
                     iY < nSrcYOff2;
                     ++iY, ++j, panLineWeight += nGaussMatrixDim )
                {
                    for( int i=0, iX = nSrcXOff; iX < nSrcXOff2; ++iX, ++i )
                    {
                        const double val =
                            pafSrcScanline[iX-nChunkXOff+(iY-nSrcYOff)
                                           * nChunkXSize];
                        if( pabySrcScanlineNodataMask == NULL ||
                            pabySrcScanlineNodataMask[iX - nChunkXOff
                                                      +(iY - nSrcYOff)
                                                      * nChunkXSize] )
                        {
                            const int nWeight = panLineWeight[i];
                            dfTotal += val * nWeight;
                            nCount += nWeight;
                        }
                    }
                }

                if( nCount == 0 )
                {
                    pafDstScanline[iDstPixel - nDstXOff] = fNoDataValue;
                }
                else
                {
                    pafDstScanline[iDstPixel - nDstXOff] =
                        static_cast<float>(dfTotal / nCount);
                }
            }
            else
            {
                int nTotalR = 0;
                int nTotalG = 0;
                int nTotalB = 0;
                int nTotalWeight = 0;
                const int *panLineWeight =
                    panGaussMatrix + nYShiftGaussMatrix * nGaussMatrixDim +
                    nXShiftGaussMatrix;

                for( int j=0, iY = nSrcYOff; iY < nSrcYOff2;
                        ++iY, ++j, panLineWeight += nGaussMatrixDim )
                {
                    for( int i=0, iX = nSrcXOff; iX < nSrcXOff2; ++iX, ++i )
                    {
                        const double val =
                            pafSrcScanline[iX - nChunkXOff +
                                           (iY-nSrcYOff) * nChunkXSize];
                        int nVal = static_cast<int>(val);
                        if( nVal >= 0 && nVal < nEntryCount &&
                            aEntries[nVal].c4 )
                        {
                            const int nWeight = panLineWeight[i];
                            nTotalR += aEntries[nVal].c1 * nWeight;
                            nTotalG += aEntries[nVal].c2 * nWeight;
                            nTotalB += aEntries[nVal].c3 * nWeight;
                            nTotalWeight += nWeight;
                        }
                    }
                }

                if( nTotalWeight == 0 )
                {
                    pafDstScanline[iDstPixel - nDstXOff] = fNoDataValue;
                }
                else
                {
                    const int nR =
                        (nTotalR + nTotalWeight / 2) / nTotalWeight;
                    const int nG =
                        (nTotalG + nTotalWeight / 2) / nTotalWeight;
                    const int nB =
                        (nTotalB + nTotalWeight / 2) / nTotalWeight;
                    pafDstScanline[iDstPixel - nDstXOff] =
                        static_cast<float>( GDALFindBestEntry(
                            nEntryCount, aEntries, nR, nG, nB ) );
                }
            }
        }

        eErr = poOverview->RasterIO(
            GF_Write, nDstXOff, iDstLine, nDstXOff2 - nDstXOff, 1,
            pafDstScanline, nDstXOff2 - nDstXOff, 1, GDT_Float32,
            0, 0, NULL );
    }

    CPLFree( pafDstScanline );
    CPLFree( aEntries );