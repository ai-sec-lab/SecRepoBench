xref_entry *compressed_entry = &ctx->xref_table->xref[compressedxrefentry->u.compressed.compressed_stream_num];
    pdf_c_stream *compressed_stream = NULL;
    pdf_c_stream *SubFile_stream = NULL;
    pdf_c_stream *Object_stream = NULL;
    char Buffer[256];
    int i = 0, object_length = 0;
    int64_t num_entries, found_object;
    int64_t Length;
    gs_offset_t offset = 0;
    pdf_stream *compressed_object = NULL;
    pdf_dict *compressed_sdict = NULL; /* alias */
    pdf_name *Type = NULL;
    pdf_obj *temp_obj;
