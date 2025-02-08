int pdfi_Do(pdf_context *pdfCtx, pdf_dict *stream_dict, pdf_dict *page_dict)
{
    int code = 0;
    pdf_name *n = NULL;
    pdf_obj *o = NULL;
    pdf_dict *sdict = NULL;
    bool known = false;

    if (pdfi_count_stack(pdfCtx) < 1) {
        code = gs_note_error(gs_error_stackunderflow);
        goto exit1;
    }
    n = (pdf_name *)pdfCtx->stack_top[-1];
    if (pdfi_type_of(n) != PDF_NAME) {
        code = gs_note_error(gs_error_typecheck);
        goto exit1;
    }
    // <MASK>
    pdfi_countdown(o);
    pdfi_pop(pdfCtx, 1);
    return code;
}