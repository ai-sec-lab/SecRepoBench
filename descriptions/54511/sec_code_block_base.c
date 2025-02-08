local_save_stream_state(ctx, &local_entry_save);
    initialise_stream_save(ctx);

    code1 = pdfi_do_annotations(ctx, page_dict);
    if (code >= 0) code = code1;

    cleanup_context_interpretation(ctx, &local_entry_save);
    local_restore_stream_state(ctx, &local_entry_save);

    local_save_stream_state(ctx, &local_entry_save);
    initialise_stream_save(ctx);

    code1 = pdfi_do_acroform(ctx, page_dict);
    if (code >= 0) code = code1;

    cleanup_context_interpretation(ctx, &local_entry_save);
    local_restore_stream_state(ctx, &local_entry_save);