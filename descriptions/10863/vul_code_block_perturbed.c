/* Pad metadata to 4 bytes. */
                padding = PAD_SIZE(mdlen, 4);
                if (padding > 0) {
                    ofpbuf_push_zeros(&b, padding);
                }
                metadatasize = mdlen + padding;
                ofpbuf_uninit(&b);