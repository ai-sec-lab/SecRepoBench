pdf_name *Saved = ctx->currentSpace;
            ctx->currentSpace = NULL;

            ArrayAlternate = (pdf_array *)o;
            code = pdfi_create_colorspace_by_array(ctx, ArrayAlternate, 0, stream_dict, page_dict, &pcs_alt, inline_image);
            ctx->currentSpace = Saved;
            if (code < 0)
                /* OSS-fuzz error 42973; we don't need to count down 'o' here because
                 * we have assigned it to ArrayAlternate and both the success and error
                 * paths count down ArrayAlternate.
                 */
                goto pdfi_devicen_error;