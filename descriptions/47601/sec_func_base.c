int pdfi_Do(pdf_context *ctx, pdf_dict *stream_dict, pdf_dict *page_dict)
{
    int code = 0;
    pdf_name *n = NULL;
    pdf_obj *o = NULL;
    pdf_dict *sdict = NULL;
    bool known = false;

    if (pdfi_count_stack(ctx) < 1) {
        code = gs_note_error(gs_error_stackunderflow);
        goto exit1;
    }
    n = (pdf_name *)ctx->stack_top[-1];
    if (pdfi_type_of(n) != PDF_NAME) {
        code = gs_note_error(gs_error_typecheck);
        goto exit1;
    }
    pdfi_countup(n);

    if (ctx->text.BlockDepth != 0)
        pdfi_set_warning(ctx, 0, NULL, W_PDF_OPINVALIDINTEXT, "pdfi_Do", NULL);

    code = pdfi_loop_detector_mark(ctx);
    if (code < 0)
        goto exit1;
    code = pdfi_find_resource(ctx, (unsigned char *)"XObject", n, (pdf_dict *)stream_dict, page_dict, &o);
    if (code < 0)
        goto exit;

    if (pdfi_type_of(o) != PDF_STREAM && pdfi_type_of(o) != PDF_DICT) {
        code = gs_note_error(gs_error_typecheck);
        goto exit;
    }

    /* This doesn't count up the stream dictionary, so we don't need to count it down later */
    code = pdfi_dict_from_obj(ctx, o, &sdict);
    if (code < 0)
        goto exit;

    code = pdfi_dict_known(ctx, sdict, "Parent", &known);
    if (code < 0)
        goto exit;
    /* Add a Parent ref, unless it happens to be a circular reference
     * (sample Bug298226.pdf -- has a circular ref and adding the Parent caused memory leak)
     */
    if (!known && sdict->object_num != stream_dict->object_num) {
        code = pdfi_dict_put(ctx, sdict, "Parent", (pdf_obj *)stream_dict);
        if (code < 0)
            goto exit;
    }

    code = pdfi_loop_detector_cleartomark(ctx);
    /* NOTE: Used to have a pdfi_gsave/pdfi_grestore around this, but it actually makes
     * things render incorrectly (and isn't in the PS code).
     * It also causes demo.ai.pdf to crash.
     * I don't really understand... (all transparency related, though, so nothing surprises me...)
     * (there are some q/Q and gsave/grestore in the code under this)
     *
     * Original Comment:
     * The image or form might change the colour space (or indeed other aspects
     * of the graphics state, if its a Form XObject. So gsave/grestore round it
     * to prevent unexpected changes.
     */
    //    pdfi_gsave(ctx);
    code = pdfi_do_image_or_form(ctx, stream_dict, page_dict, o);
    //    pdfi_grestore(ctx);
    pdfi_countdown(n);
    pdfi_countdown(o);
    pdfi_pop(ctx, 1);
    return code;

exit:
    (void)pdfi_loop_detector_cleartomark(ctx);
exit1:
    pdfi_countdown(n);
    pdfi_countdown(o);
    pdfi_pop(ctx, 1);
    return code;
}