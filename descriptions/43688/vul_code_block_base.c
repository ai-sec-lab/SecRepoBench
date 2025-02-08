if (ctx->page.has_transparency && page_group_known) {
        code1 = pdfi_trans_end_group(ctx);
        if (code >= 0)
            code = code1;
    }

    if (need_pdf14) {
        if (code < 0) {
            (void)gs_abort_pdf14trans_device(ctx->pgs);
            goto exit1;
        }

        code = gs_pop_pdf14trans_device(ctx->pgs, false);
        if (code < 0) {
            goto exit1;
        }
    }

exit1:
    pdfi_free_DefaultQState(ctx);
    pdfi_grestore(ctx);

exit2:
    pdfi_countdown(ctx->page.CurrentPageDict);
    ctx->page.CurrentPageDict = NULL;

exit3:
    pdfi_countdown(page_dict);
    pdfi_countdown(group_dict);

    release_page_DefaultSpaces(ctx);

    if (code == 0 || (!ctx->args.pdfstoponerror && code != gs_error_stackoverflow))
        if (!page_dict_error && ctx->finish_page != NULL)
            code = ctx->finish_page(ctx);
    return code;