static int pdfi_read_stream_object(pdf_context *ctx, pdf_c_stream *s, gs_offset_t stream_offset,
                                   uint32_t objnum, uint32_t gen)
{
    int code = 0;
    int64_t i;
    pdf_keyword *keyword = NULL;
    pdf_dict *dict = NULL;
    gs_offset_t offset;
    pdf_stream *stream_obj = NULL;

    /* Strange code time....
     * If we are using a stream which is *not* the PDF uncompressed main file stream
     * then doing stell on it will only tell us how many bytes have been read from
     * that stream, it won't tell us the underlying file position. So we add on the
     * 'unread' bytes, *and* we add on the position of the start of the stream in
     * the actual main file. This is all done so that we can check the /Length
     * of the object. Note that this will *only* work for regular objects it can
     * not be used for compressed object streams, but those don't need checking anyway
     * they have a different mechanism altogether and should never get here.
     */
    offset = stell(s->s) - s->unread_size + stream_offset;
    code = pdfi_seek(ctx, ctx->main_stream, offset, SEEK_SET);

    if (pdfi_count_stack(ctx) < 1)
        return_error(gs_error_stackunderflow);

    dict = (pdf_dict *)ctx->stack_top[-1];
    dict->indirect_num = dict->object_num = objnum;
    dict->indirect_gen = dict->generation_num = gen;

    if (dict->type != PDF_DICT) {
        pdfi_pop(ctx, 1);
        return_error(gs_error_syntaxerror);
    }

    /* Convert the dict into a stream */
    code = pdfi_obj_dict_to_stream(ctx, dict, &stream_obj, true);
    if (code < 0) {
        pdfi_pop(ctx, 1);
        return code;
    }
    /* Pop off the dict and push the stream */
    pdfi_pop(ctx, 1);
    dict = NULL;
    // <MASK> /* get rid of extra ref */

    keyword = ((pdf_keyword *)ctx->stack_top[-1]);
    if (keyword->key != TOKEN_ENDOBJ) {
        pdfi_pop(ctx, 2);
        return_error(gs_error_typecheck);
    }
    pdfi_pop(ctx, 1);
    return 0;
}