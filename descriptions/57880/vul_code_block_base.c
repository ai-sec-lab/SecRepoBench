if (toffs == 0 || tlen == 0 || toffs + tlen > fbuflen) {
            gs_free_object(ctx->memory, pfbuf, "pdfi_read_cff_font(fbuf)");
            return_error(gs_error_invalidfont);
        }
        fbuf += toffs;
        fbuflen = tlen;