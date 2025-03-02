FORCE_INLINE int Lizard_decompress_LIZv1(
                 Lizard_dstream_t* ctx,
                 BYTE* const dest,
                 int outputSize,         /* this value is the max size of Output Buffer. */

                 int partialDecoding,    /* full, partial */
                 int targetOutputSize,   /* only used if partialDecoding==partial */
                 int dict,               /* noDict, withPrefix64k, usingExtDict */
                 const BYTE* const lowPrefix,  /* == dest if dict == noDict */
                 const BYTE* const dictStart,  /* only if dict==usingExtDict */
                 const size_t dictSize,         /* note : = 0 if noDict */
                 int compressionLevel
                 )
{
    /* Local Variables */
    int inputSize = (int)(ctx->flagsEnd - ctx->flagsPtr);
    const BYTE* const blockBase = ctx->flagsPtr;
    const BYTE* const iend = ctx->literalsEnd;

    BYTE* op = dest;
    BYTE* const oend = op + outputSize;
    BYTE* cpy = NULL;
    BYTE* oexit = op + targetOutputSize;
    const BYTE* const lowLimit = lowPrefix - dictSize;
    const BYTE* const dictEnd = (const BYTE*)dictStart + dictSize;

    const int checkOffset = (dictSize < (int)(LIZARD_DICT_SIZE));

    intptr_t last_off = ctx->last_off;
    intptr_t length = 0;
    (void)compressionLevel;

    /* Special cases */
    if (unlikely(outputSize==0)) return ((inputSize==1) && (*ctx->flagsPtr==0)) ? 0 : -1;  /* Empty output buffer */

    /* Main Loop : decode sequences */
    while (ctx->flagsPtr < ctx->flagsEnd) {
        unsigned token;
        const BYTE* match;
    //    intptr_t litLength;

        if ((partialDecoding) && (op >= oexit)) return (int) (op-dest); 

        /* get literal length */
        token = *ctx->flagsPtr++;

        if (token >= 32)
        {
            // <MASK>
                if (unlikely((size_t)(ctx->literalsPtr+length)<(size_t)(ctx->literalsPtr))) { LIZARD_LOG_DECOMPRESS_LIZv1("3"); goto _output_error; }   /* overflow detection */
            }

            /* copy literals */
            cpy = op + length;
            if (unlikely(cpy > oend - WILDCOPYLENGTH || ctx->literalsPtr > iend - WILDCOPYLENGTH)) { LIZARD_LOG_DECOMPRESS_LIZv1("offset outside buffers\n"); goto _output_error; }   /* Error : offset outside buffers */
    #if 1
            Lizard_wildCopy16(op, ctx->literalsPtr, cpy);
            op = cpy;
            ctx->literalsPtr += length; 
    #else
            Lizard_copy8(op, ctx->literalsPtr);
            Lizard_copy8(op+8, ctx->literalsPtr+8);
            if (length > 16)
                Lizard_wildCopy16(op + 16, ctx->literalsPtr + 16, cpy);
            op = cpy;
            ctx->literalsPtr += length; 
    #endif

            /* get offset */
            if (unlikely(ctx->offset16Ptr > ctx->offset16End - 2)) { LIZARD_LOG_DECOMPRESS_LIZv1("(ctx->offset16Ptr > ctx->offset16End\n"); goto _output_error; }
#if 1
            { /* branchless */
                intptr_t new_off = MEM_readLE16(ctx->offset16Ptr);
                uintptr_t not_repCode = (uintptr_t)(token >> ML_RUN_BITS) - 1;
                last_off ^= not_repCode & (last_off ^ -new_off);
                ctx->offset16Ptr = (BYTE*)((uintptr_t)ctx->offset16Ptr + (not_repCode & 2));
            }
#else
            if ((token >> ML_RUN_BITS_LIZv1) == 0)
            {
                last_off = -(intptr_t)MEM_readLE16(ctx->offset16Ptr); 
                ctx->offset16Ptr += 2;
            }
#endif

            /* get matchlength */
            length = (token >> RUN_BITS_LIZv1) & MAX_SHORT_MATCHLEN;
            if (length == MAX_SHORT_MATCHLEN) {
                if (unlikely(ctx->literalsPtr > iend - 1)) { LIZARD_LOG_DECOMPRESS_LIZv1("6"); goto _output_error; } 
                length = *ctx->literalsPtr;
                if unlikely(length >= 254) {
                    if (length == 254) {
                        length = MEM_readLE16(ctx->literalsPtr+1);
                        ctx->literalsPtr += 2;
                    } else {
                        length = MEM_readLE24(ctx->literalsPtr+1);
                        ctx->literalsPtr += 3;
                    }
                }
                length += MAX_SHORT_MATCHLEN;
                ctx->literalsPtr++;
                if (unlikely((size_t)(op+length)<(size_t)(op))) { LIZARD_LOG_DECOMPRESS_LIZv1("7"); goto _output_error; }  /* overflow detection */
            }

            DECOMPLOG_CODEWORDS_LIZv1("T32+ literal=%u match=%u offset=%d ipos=%d opos=%d\n", (U32)litLength, (U32)length, (int)-last_off, (U32)(ctx->flagsPtr-blockBase), (U32)(op-dest));
        }