if( i < nNumLayers_ - 1 )
            {
                memcpy( papoLayers_ + i, papoLayers_ + i + 1,
                        (nNumLayers_ - 1 - i) * sizeof(KMLNode*) );
            }
            nNumLayers_ --;
            continue;