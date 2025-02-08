static int pdfi_annot_draw(pdf_context *ctx, pdf_dict *annot, pdf_name *subtype)
{
    pdf_obj *NormAP = NULL;
    int code = 0;
    annot_dispatch_t *dispatch_ptr;
    bool rendercompleted = true;

    /* See if annotation is visible */
    if (!pdfi_annot_visible(ctx, annot, subtype))
        goto exit;

    /* See if we are rendering this type of annotation */
    if (!pdfi_annot_check_type(ctx, subtype))
        goto exit;

    /* Get the Normal AP, if it exists */
    code = pdfi_annot_get_NormAP(ctx, annot, &NormAP);
    if (code < 0) goto exit;

    code = pdfi_gsave(ctx);
    if (code < 0) goto exit;

    /* Draw the annotation */
    for (dispatch_ptr = annot_dispatch; dispatch_ptr->subtype; dispatch_ptr ++) {
        if (pdfi_name_is(subtype, dispatch_ptr->subtype)) {
            if (NormAP && dispatch_ptr->simpleAP)
                rendercompleted = false;
            else
                code = dispatch_ptr->func(ctx, annot, NormAP, &rendercompleted);
            break;
        }
    }
    if (!dispatch_ptr->subtype) {
        char str[100];
        memcpy(str, (const char *)subtype->data, subtype->length < 100 ? subtype->length : 99);
        str[subtype->length < 100 ? subtype->length : 99] = '\0';
        dbgmprintf1(ctx->memory, "ANNOT: No handler for subtype %s\n", str);

        /* Not necessarily an error? We can just render the AP if there is one */
        rendercompleted = false;
    }

    if (!rendercompleted)
        code = pdfi_annot_draw_AP(ctx, annot, NormAP);

    (void)pdfi_grestore(ctx);

 exit:
    pdfi_countdown(NormAP);
    return code;
}