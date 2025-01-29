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