pdf_name *Saved = ctx->currentSpace;
            ctx->currentSpace = NULL;

            ArrayAlternate = (pdf_array *)o;
            statuscode = pdfi_create_colorspace_by_array(ctx, ArrayAlternate, 0, stream_dict, page_dict, &pcs_alt, inline_image);
            ctx->currentSpace = Saved;
            if (statuscode < 0) {
                pdfi_countdown(o);
                goto pdfi_devicen_error;
            }