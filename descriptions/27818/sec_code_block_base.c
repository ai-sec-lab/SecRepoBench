if ((length=(token & MAX_SHORT_LITLEN)) == MAX_SHORT_LITLEN) {
                if (unlikely(ctx->literalsPtr > iend - 1)) { LIZARD_LOG_DECOMPRESS_LIZv1("1"); goto _output_error; } 
                length = *ctx->literalsPtr;
                ctx->literalsPtr++;
                if unlikely(length >= 254) {
                    if (length == 254) {
                        if (unlikely(ctx->literalsPtr > iend - 2)) { LIZARD_LOG_DECOMPRESS_LIZv1("1"); goto _output_error; }  /* overflow detection */
                        length = MEM_readLE16(ctx->literalsPtr);
                        ctx->literalsPtr += 2;
                    } else {
                        if (unlikely(ctx->literalsPtr > iend - 3)) { LIZARD_LOG_DECOMPRESS_LIZv1("1"); goto _output_error; }  /* overflow detection */
                        length = MEM_readLE24(ctx->literalsPtr);
                        ctx->literalsPtr += 3;
                    }
                }
                length += MAX_SHORT_LITLEN;
                if (unlikely((size_t)(op+length)<(size_t)(op))) { LIZARD_LOG_DECOMPRESS_LIZv1("2"); goto _output_error; }  /* overflow detection */