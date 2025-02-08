IntergraphRLEBand::IntergraphRLEBand( IntergraphDataset *poDSIn,
                                      int nBandIn,
                                      int nBandOffset,
                                      int nColorChannel ) :
    IntergraphRasterBand( poDSIn, nBandIn, nBandOffset ),
    pabyRLEBlock(NULL),
    nRLESize(0),
    bRLEBlockLoaded(FALSE),
    panRLELineOffset(NULL)
{
    nRGBIndex = static_cast<uint8>(nColorChannel);

    if( pabyBlockBuf == NULL )
        return;

    if( !bTiled )
    {
        // ------------------------------------------------------------
        // Load all rows at once
        // ------------------------------------------------------------

        nFullBlocksX = 1;

        if( eFormat == RunLengthEncodedC || eFormat == RunLengthEncoded )
        {
            nBlockYSize = 1;
            if( nRasterYSize > 1024 * 1024 )
            {
                VSIFSeekL( poDSIn->fp, 0, SEEK_END );
                // At the very least 2 bytes per row (probably more)
                if( VSIFTellL( poDSIn->fp ) / 2 <
                                    static_cast<vsi_l_offset>(nRasterYSize) )
                {
                    CPLError(CE_Failure, CPLE_AppDefined, "File too short");
                    return;
                }
            }
            panRLELineOffset = (uint32 *)
                VSI_CALLOC_VERBOSE(sizeof(uint32),nRasterYSize);
            if( panRLELineOffset == NULL )
                return;
            nFullBlocksY = nRasterYSize;
        }
        else
        {
            nBlockYSize  = nRasterYSize;
            nFullBlocksY = 1;
        }

        nRLESize = INGR_GetDataBlockSize( poDSIn->pszFilename,
                                          hHeaderTwo.CatenatedFilePointer,
                                          nDataOffset);

        if( nBlockYSize == 0 || nBlockXSize > INT_MAX / nBlockYSize )
        {
            CPLError(CE_Failure, CPLE_AppDefined, "Too big block size");
            return;
        }
        nBlockBufSize = nBlockXSize * nBlockYSize;
    }
    else
    {
        // ------------------------------------------------------------
        // Find the biggest tile
        // ------------------------------------------------------------

        for( uint32 iTiles = 0; iTiles < nTiles; iTiles++)
        {
            nRLESize = MAX( pahTiles[iTiles].Used, nRLESize );
        }
    }

    // ----------------------------------------------------------------
    // Reallocate the decompressed buffer.
    // ----------------------------------------------------------------

    if( eFormat == AdaptiveRGB ||
        eFormat == ContinuousTone )
    {
        if( nBlockBufSize > INT_MAX / 3 )
        {
            CPLError(CE_Failure, CPLE_AppDefined, "Too big block size");
            return;
        }
        nBlockBufSize *= 3;
    }

    CPLFree(pabyBlockBuf);
    pabyBlockBuf = NULL;
    if( nBlockBufSize > 0 )
        pabyBlockBuf = (GByte*) VSIMalloc( nBlockBufSize );
    if (pabyBlockBuf == NULL)
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Cannot allocate %d bytes", nBlockBufSize);
    }

    // ----------------------------------------------------------------
    // Create a RLE buffer
    // ----------------------------------------------------------------

    if( nRLESize == 0 )
        pabyRLEBlock = (GByte*) VSIMalloc( 1 );
    else if( nRLESize < INT_MAX )
        pabyRLEBlock = (GByte*) VSIMalloc( nRLESize );
    if (pabyRLEBlock == NULL)
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Cannot allocate %d bytes", nRLESize);
    }

    // ----------------------------------------------------------------
    // Set a black and white Color Table
    // ----------------------------------------------------------------

    if( eFormat == RunLengthEncoded )
    {
        BlackWhiteCT( true );
    }
}