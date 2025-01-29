if( index < nNumLayers_ - 1 )
            {
                memmove( papoLayers_ + index, papoLayers_ + index + 1,
                        (nNumLayers_ - 1 - index) * sizeof(KMLNode*) );
            }
            nNumLayers_ --;
            continue;