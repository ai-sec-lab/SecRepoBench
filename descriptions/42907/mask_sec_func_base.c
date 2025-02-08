int pdfi_read_type0_font(pdf_context *ctx, pdf_dict *font_dict, pdf_dict *stream_dict, pdf_dict *page_dict, pdf_font **ppdffont)
{
    int code, nlen;
    pdf_obj *cmap = NULL;
    pdf_cmap *pcmap = NULL;
    pdf_array *arr = NULL;
    pdf_dict *decfontdict = NULL; /* there can only be one */
    pdf_name *n = NULL;
    pdf_obj *basefont = NULL;
    pdf_obj *tounicode = NULL;
    pdf_dict *dfontdesc = NULL;
    pdf_dict *fontdesc = NULL;
    pdf_stream *ffile = NULL;
    pdf_font *descpfont = NULL;
    pdf_font_type0 *pdft0 = NULL;
    gs_font_type0 *pfont0 = NULL;
    pdfi_cid_decoding_t *dec = NULL;
    pdfi_cid_subst_nwp_table_t *substnwp = NULL;

    /* We're supposed to have a FontDescriptor, it can be missing, and we have to carry on */
    (void)pdfi_dict_get(ctx, font_dict, "FontDescriptor", (pdf_obj **)&fontdesc);

    code = pdfi_dict_get(ctx, font_dict, "Encoding", &cmap);
    if (code < 0) goto error;

    if (cmap->type == PDF_CMAP) {
        pcmap = (pdf_cmap *)cmap;
        cmap = NULL;
    }
    else {
        code = pdfi_read_cmap(ctx, cmap, &pcmap);
        pdfi_countdown(cmap);
        cmap = NULL;
        if (code < 0) goto error;
    }

    code = pdfi_dict_get(ctx, font_dict, "DescendantFonts", (pdf_obj **)&arr);
    if (code < 0) goto error;

    if (arr->type != PDF_ARRAY || arr->size != 1) {
        code = gs_note_error(gs_error_invalidfont);
        goto error;
    }
    code = pdfi_array_get(ctx, arr, 0, (pdf_obj **)&decfontdict);
    pdfi_countdown(arr);
    arr = NULL;
    if (code < 0) goto error;
    if (decfontdict->type == PDF_FONT) {
        descpfont = (pdf_font *)decfontdict;
        decfontdict = descpfont->PDF_font;
        pdfi_countup(decfontdict);
    }
    else {
        if (decfontdict->type != PDF_DICT) {
            code = gs_note_error(gs_error_invalidfont);
            goto error;
        }
        code = pdfi_dict_get(ctx, (pdf_dict *)decfontdict, "Type", (pdf_obj **)&n);
        if (code < 0) goto error;
        if (n->type != PDF_NAME || n->length != 4 || memcmp(n->data, "Font", 4) != 0) {
            pdfi_countdown(n);
            code = gs_note_error(gs_error_invalidfont);
            goto error;
        }
        pdfi_countdown(n);
    }
#if 0
    code = pdfi_dict_get(ctx, (pdf_dict *)decfontdict, "Subtype", (pdf_obj **)&n);
    if (code < 0)
        goto error;

    if (n->type != PDF_NAME || n->length != 12 || memcmp(n->data, "CIDFontType", 11) != 0) {
        pdfi_countdown(n);
        code = gs_note_error(gs_error_invalidfont);
        goto error;
    }
    /* cidftype is ignored for now, but we may need to know it when
       subsitutions are allowed
     */
    cidftype = n->data[11] - 48;

    pdfi_countdown(n);
#endif

    code = pdfi_dict_get(ctx, font_dict, "BaseFont", (pdf_obj **)&basefont);
    if (code < 0) {
        basefont = NULL;
    }

    if (ctx->args.ignoretounicode != true) {
        code = pdfi_dict_get(ctx, font_dict, "ToUnicode", (pdf_obj **)&tounicode);
        if (code >= 0 && tounicode->type == PDF_STREAM) {
            pdf_cmap *tu = NULL;
            code = pdfi_read_cmap(ctx, tounicode, &tu);
            pdfi_countdown(tounicode);
            tounicode = (pdf_obj *)tu;
        }
        if (code < 0 || (tounicode != NULL && tounicode->type != PDF_CMAP)) {
            pdfi_countdown(tounicode);
            tounicode = NULL;
            code = 0;
        }
    }
    else {
        tounicode = NULL;
    }

    if (descpfont == NULL) {
        gs_font *pf;

        code = pdfi_load_font(ctx, stream_dict, page_dict, decfontdict, &pf, true);
        if (code < 0)
            goto error;
        descpfont = (pdf_font *)pf->client_data;
    }

    // <MASK>
}