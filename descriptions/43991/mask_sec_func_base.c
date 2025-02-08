int
pdfi_setcolorN(pdf_context *ctx, pdf_dict *stream_dict, pdf_dict *page_dict, bool is_fill)
{
    gs_color_space *pcs;
    gs_color_space *base_space = NULL;
    int ncomps=0, code = 0;
    gs_client_color cc;
    bool is_pattern = false;

    if (!is_fill) {
        gs_swapcolors_quick(ctx->pgs);
    }
    pcs = gs_currentcolorspace(ctx->pgs);

    // <MASK>
    return code;
}