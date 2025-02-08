code = pdfi_merge_dicts(ctx, ctx->Trailer, sdict);
        if (code < 0) {
            if (code == gs_error_VMerror || ctx->args.pdfstoponerror)
                return code;
        }