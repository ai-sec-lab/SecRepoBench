if( nTileBytes > nMaxBlockBytes )
            {
                CPLDebug("RMF",
                         "Only reading %u bytes instead of the %u declared "
                         "in the tile array",
                         nMaxBlockBytes, nTileBytes);
                nTileBytes = nMaxBlockBytes;
            }

            GByte *pabyNewTile = reinterpret_cast<GByte *>(
                    VSIRealloc(poGDS->pabyCurrentTile, std::max(1U, nTileBytes)));
            if( !pabyNewTile )
            {
                CPLError( CE_Failure, CPLE_FileIO,
                          "Can't allocate tile block of size %lu.\n%s",
                          (unsigned long)nTileBytes, VSIStrerror( errno ) );
                poGDS->nCurrentTileBytes = 0;
                return CE_Failure;
            }