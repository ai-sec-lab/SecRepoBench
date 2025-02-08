
    /* Required */
    code = pdfi_dict_get_number2(ctx, image_dict, "Width", "W", &temp_f);
    if (code < 0)
        goto errorExit;
    info->Width = (int)temp_f;
    if ((int)temp_f != (int)(temp_f+.5)) {
        pdfi_set_warning(ctx, 0, NULL, W_PDF_BAD_IMAGEDICT, "pdfi_get_image_info", NULL);
        if (ctx->args.pdfstoponwarning) {
            code = gs_note_error(gs_error_rangecheck);
            goto errorExit;
        }
    }