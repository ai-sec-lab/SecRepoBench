static int pdfi_process_one_page(pdf_context *ctx, pdf_dict *pagedictionary)
{
    stream_save local_entry_save;
    int code, code1;

    /* Save the current stream state, for later cleanup, in a local variable */
    local_save_stream_state(ctx, &local_entry_save);
    initialise_stream_save(ctx);

    code = pdfi_process_page_contents(ctx, pagedictionary);

    /* Put our state back the way it was before we ran the contents
     * and check if the stream had problems
     */
#if PROBE_STREAMS
    if (ctx->pgs->level > ctx->current_stream_save.gsave_level ||
        pdfi_count_stack(ctx) > ctx->current_stream_save.stack_count)
        code = ((pdf_context *)0)->first_page;
#endif

    cleanup_context_interpretation(ctx, &local_entry_save);
    local_restore_stream_state(ctx, &local_entry_save);

    local_save_stream_state(ctx, &local_entry_save);
    initialise_stream_save(ctx);

    code1 = pdfi_do_annotations(ctx, pagedictionary);
    if (code >= 0) code = code1;

    cleanup_context_interpretation(ctx, &local_entry_save);
    local_restore_stream_state(ctx, &local_entry_save);

    local_save_stream_state(ctx, &local_entry_save);
    initialise_stream_save(ctx);

    code1 = pdfi_do_acroform(ctx, pagedictionary);
    if (code >= 0) code = code1;

    cleanup_context_interpretation(ctx, &local_entry_save);
    local_restore_stream_state(ctx, &local_entry_save);

    if (ctx->text.BlockDepth != 0) {
        pdfi_set_warning(ctx, 0, NULL, W_PDF_UNBLANACED_BT, "pdfi_process_one_page", "");
        ctx->text.BlockDepth = 0;
    }
    return code;
}