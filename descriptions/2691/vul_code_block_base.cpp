panRLELineOffset = (uint32 *)
                VSI_CALLOC_VERBOSE(sizeof(uint32),nRasterYSize);
            if( panRLELineOffset == NULL )
                return;
            nFullBlocksY = nRasterYSize;