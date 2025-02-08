int pdfi_Tj(pdf_context *ctx)
{
    int code = 0;
    pdf_string *s = NULL;
    gs_matrix saved, Trm;
    gs_point initial_point, currpt, pt;
    double linewidth = ctx->pgs->line_params.half_width;

    if (pdfi_count_stack(ctx) < 1)
        return_error(gs_error_stackunderflow);

    if (pdfi_oc_is_off(ctx))
        goto exit;

    s = (pdf_string *)ctx->stack_top[-1];
    if (s->type != PDF_STRING)
        return_error(gs_error_typecheck);

    /* We can't rely on the stack reference because an error during
       the text operation (i.e. retrieving objects for glyph metrics
       may cause the stack to be cleared.
     */
    // <MASK>
    return code;
}