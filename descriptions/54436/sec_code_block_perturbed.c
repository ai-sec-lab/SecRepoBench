code = pdfi_array_get_int(ctx, a, (uint64_t)i, (int64_t *)&W[i]);
        if (code < 0 || W[i] < 0) {
            pdfi_countdown(a);
            pdfi_close_file(ctx, XRefStrm);
            pdfi_countdown(ctx->xref_table);
            ctx->xref_table = NULL;
            if (W[i] < 0)
                code = gs_note_error(gs_error_rangecheck);
            return code;
        }