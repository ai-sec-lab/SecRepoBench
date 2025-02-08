code = pdfi_build_function(ctx, &gsfunc, NULL, 1, TR, NULL);
                if (code < 0)
                    goto exit;
                if (gsfunc->params.m != 1 || gsfunc->params.n != 1) {
                    pdfi_free_function(ctx, gsfunc);
                    gsfunc = NULL;
                    dmprintf(ctx->memory, "WARNING: Ignoring invalid TR (number of inpuits or outputs not 1) in SMask\n");
                }