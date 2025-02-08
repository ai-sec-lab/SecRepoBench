pdf_name *Type = NULL;
    pdf_obj *temp_obj;

    if (entry->u.compressed.compressed_stream_num > ctx->xref_table->xref_size)
        return_error(gs_error_undefined);

    compressed_entry = &ctx->xref_table->xref[entry->u.compressed.compressed_stream_num];