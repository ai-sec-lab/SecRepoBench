int pdfi_open_memory_stream_from_filtered_stream(pdf_context *ctx, pdf_stream *stream_obj,
                                                 unsigned int size, byte **Buffer, pdf_c_stream *source,
                                                 pdf_c_stream **new_pdf_stream, bool retain_ownership)
{
    int code;
    pdf_dict *dict = NULL;
    int decompressed_length = 0;
    byte *decompressed_Buffer = NULL;
    pdf_c_stream *compressed_stream = NULL, *decompressed_stream = NULL;
    bool known = false;

    code = pdfi_open_memory_stream_from_stream(ctx, (unsigned int)size, Buffer, source, new_pdf_stream, retain_ownership);
    if (code < 0) {
        pdfi_close_memory_stream(ctx, *Buffer, *new_pdf_stream);
        *Buffer = NULL;
        *new_pdf_stream = NULL;
        return code;
    }

    if (stream_obj == NULL)
        return size;

    code = pdfi_dict_from_obj(ctx, (pdf_obj *)stream_obj, &dict);
    if (code < 0)
        return code;

    pdfi_dict_known(ctx, dict, "F", &known);
    if (!known)
        pdfi_dict_known(ctx, dict, "Filter", &known);

    if (!known)
        return size;

    compressed_stream = *new_pdf_stream;
    /* This is again complicated by requiring a seekable stream, and the fact that,
     * unlike fonts, there is no Length2 key to tell us how large the uncompressed
     * stream is.
     */
    // <MASK>
}