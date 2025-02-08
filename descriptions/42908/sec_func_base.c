bool
pdfi_oc_is_ocg_visible(pdf_context *ctx, pdf_dict *ocdict)
{
    pdf_name *type = NULL;
    bool is_visible = true;
    int code;

    /* Type can be either OCMD or OCG.
     */
    code = pdfi_dict_knownget_type(ctx, ocdict, "Type", PDF_NAME, (pdf_obj **)&type);
    if (code <= 0)
        goto cleanup;

    if (pdfi_name_is(type, "OCMD")) {
        is_visible = pdfi_oc_check_OCMD(ctx, ocdict);
    } else if (pdfi_name_is(type, "OCG")) {
        is_visible = pdfi_get_default_OCG_val(ctx, ocdict);
        if (is_visible)
            is_visible = pdfi_oc_check_OCG_usage(ctx, ocdict);
    } else {
        char str[100];
        memcpy(str, (const char *)type->data, type->length < 100 ? type->length : 99);
        str[type->length < 100 ? type->length : 99] = '\0';
        dmprintf1(ctx->memory, "WARNING: OC dict type is %s, expected OCG or OCMD\n", str);
    }

 cleanup:
    pdfi_countdown(type);

    if (ctx->args.pdfdebug) {
        dmprintf2(ctx->memory, "OCG: OC Dict %d %s visible\n", ocdict->object_num,
                  is_visible ? "IS" : "IS NOT");
    }
    return is_visible;
}