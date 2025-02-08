strm = push_pfb_filter(ctx->memory, inbuf, inbuf + inlen);
    if (strm == NULL) {
        code = gs_note_error(gs_error_VMerror);
    }
    else {
        while (1) {
            c = sgetc(strm);
            if (c < 0)
                break;
            decodedlength++;
        }
        pop_pfb_filter(ctx->memory, strm);
        decodebuf = gs_alloc_bytes(ctx->memory, decodedlength, "pdfi_t1_decode_pfb(decodebuf)");
        if (decodebuf == NULL) {
            code = gs_note_error(gs_error_VMerror);
        }
        else {
            d = decodebuf;
            strm = push_pfb_filter(ctx->memory, inbuf, inbuf + inlen);
            while (1) {
                c = sgetc(strm);
                if (c < 0)
                    break;
                *d = c;
                d++;
            }
            pop_pfb_filter(ctx->memory, strm);
            *outbuf = decodebuf;
            *outlen = decodedlength;
        }
    }
    return code;