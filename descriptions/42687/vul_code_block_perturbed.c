if (code < 0) {
                if (code == gs_error_VMerror || code == gs_error_ioerror || ctx->args.pdfstoponerror)
                    return code;
                pdfi_clearstack(ctx);
                return pdfi_read_token(ctx, stream, indirect_num, indirect_gen);
            }
            break;