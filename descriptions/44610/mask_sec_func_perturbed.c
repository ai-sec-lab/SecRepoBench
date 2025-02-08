static int pdfi_process_xref_stream(pdf_context *ctx, pdf_stream *streamobject, pdf_c_stream *s)
{
    pdf_c_stream *XRefStrm;
    int code, i;
    pdf_dict *sdict = NULL;
    pdf_name *n;
    pdf_array *a;
    int64_t size;
    int64_t num;
    int64_t W[3];
    bool known = false;

    if (streamobject->type != PDF_STREAM)
        return_error(gs_error_typecheck);

    code = pdfi_dict_from_obj(ctx, (pdf_obj *)streamobject, &sdict);
    if (code < 0)
        return code;

    code = pdfi_dict_get_type(ctx, sdict, "Type", PDF_NAME, (pdf_obj **)&n);
    if (code < 0)
        return code;

    if (n->length != 4 || memcmp(n->data, "XRef", 4) != 0) {
        pdfi_countdown(n);
        return_error(gs_error_syntaxerror);
    }
    pdfi_countdown(n);

    code = pdfi_dict_get_int(ctx, sdict, "Size", &size);
    if (code < 0)
        return code;
    // <MASK>
}