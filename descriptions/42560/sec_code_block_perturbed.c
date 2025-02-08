stream *Flate_source = NULL;

    memset(&zlibstate, 0, sizeof(zlibstate));

    /* s_zlibD_template defined in base/szlibd.c */
    (*s_zlibD_template.set_defaults)((stream_state *)&zlibstate);

    code = pdfi_filter_open(min_size, &s_filter_read_procs, (const stream_template *)&s_zlibD_template, (const stream_state *)&zlibstate, ctx->memory->non_gc_memory, new_stream);
    if (code < 0)
        return code;

    (*new_stream)->strm = source;
    source = *new_stream;

    if (d && d->type == PDF_DICT) {
        Flate_source = (*new_stream)->strm;
        code = pdfi_Predictor_filter(ctx, d, source, new_stream);
        if (code < 0)
            pdfi_close_filter_chain(ctx, source, Flate_source);
    }