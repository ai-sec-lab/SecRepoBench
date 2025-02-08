static int pdfi_process_xref_stream(pdf_context *ctx, pdf_stream *stream_obj, pdf_c_stream *s)
{
    pdf_c_stream *XRefStrm;
    int code, i;
    pdf_dict *sdict = NULL;
    pdf_name *n;
    pdf_array *a;
    int64_t size;
    int64_t num;
    int64_t W[3];
    int objnum;
    bool isknown = false;

    if (pdfi_type_of(stream_obj) != PDF_STREAM)
        return_error(gs_error_typecheck);

    code = pdfi_dict_from_obj(ctx, (pdf_obj *)stream_obj, &sdict);
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
    if (size < 1)
        return 0;

    if (size < 0 || size > floor((double)ARCH_MAX_SIZE_T / (double)sizeof(xref_entry)))
        return_error(gs_error_rangecheck);

    /* If this is the first xref stream then allocate the xref table and store the trailer */
    if (ctx->xref_table == NULL) {
        ctx->xref_table = (xref_table_t *)gs_alloc_bytes(ctx->memory, sizeof(xref_table_t), "read_xref_stream allocate xref table");
        if (ctx->xref_table == NULL) {
            return_error(gs_error_VMerror);
        }
        memset(ctx->xref_table, 0x00, sizeof(xref_table_t));
        ctx->xref_table->xref = (xref_entry *)gs_alloc_bytes(ctx->memory, size * sizeof(xref_entry), "read_xref_stream allocate xref table entries");
        if (ctx->xref_table->xref == NULL){
            gs_free_object(ctx->memory, ctx->xref_table, "failed to allocate xref table entries");
            ctx->xref_table = NULL;
            return_error(gs_error_VMerror);
        }
        memset(ctx->xref_table->xref, 0x00, size * sizeof(xref_entry));
        ctx->xref_table->ctx = ctx;
        ctx->xref_table->type = PDF_XREF_TABLE;
        ctx->xref_table->xref_size = size;
#if REFCNT_DEBUG
        ctx->xref_table->UID = ctx->ref_UID++;
        dmprintf1(ctx->memory, "Allocated xref table with UID %"PRIi64"\n", ctx->xref_table->UID);
#endif
        pdfi_countup(ctx->xref_table);

        ctx->Trailer = sdict;
        pdfi_countup(sdict);
    } else {
        if (size > ctx->xref_table->xref_size)
            return_error(gs_error_rangecheck);

        code = pdfi_merge_dicts(ctx, ctx->Trailer, sdict);
        if (code < 0) {
            if (code == gs_error_VMerror || ctx->args.pdfstoponerror)
                return code;
        }
    }

    pdfi_seek(ctx, ctx->main_stream, pdfi_stream_offset(ctx, stream_obj), SEEK_SET);

    /* Bug #691220 has a PDF file with a compressed XRef, the stream dictionary has
     * a /DecodeParms entry for the stream, which has a /Colors value of 5, which makes
     * *no* sense whatever. If we try to apply a Predictor then we end up in a loop trying
     * to read 5 colour samples. Rather than meddles with more parameters to the filter
     * code, we'll just remove the Colors entry from the DecodeParms dictionary,
     * because it is nonsense. This means we'll get the (sensible) default value of 1.
     */
    code = pdfi_dict_known(ctx, sdict, "DecodeParms", &isknown);
    if (code < 0)
        return code;

    if (isknown) {
        pdf_dict *DP;
        double f;
        pdf_obj *name;

        code = pdfi_dict_get_type(ctx, sdict, "DecodeParms", PDF_DICT, (pdf_obj **)&DP);
        if (code < 0)
            return code;

        code = pdfi_dict_knownget_number(ctx, DP, "Colors", &f);
        if (code < 0) {
            pdfi_countdown(DP);
            return code;
        }
        if (code > 0 && f != (double)1)
        {
            code = pdfi_name_alloc(ctx, (byte *)"Colors", 6, &name);
            if (code < 0) {
                pdfi_countdown(DP);
                return code;
            }
            pdfi_countup(name);

            code = pdfi_dict_delete_pair(ctx, DP, (pdf_name *)name);
            pdfi_countdown(name);
            if (code < 0) {
                pdfi_countdown(DP);
                return code;
            }
        }
        pdfi_countdown(DP);
    }

    code = pdfi_filter_no_decryption(ctx, stream_obj, s, &XRefStrm, false);
    if (code < 0) {
        pdfi_countdown(ctx->xref_table);
        ctx->xref_table = NULL;
        return code;
    }

    code = pdfi_dict_get_type(ctx, sdict, "W", PDF_ARRAY, (pdf_obj **)&a);
    if (code < 0) {
        pdfi_close_file(ctx, XRefStrm);
        pdfi_countdown(ctx->xref_table);
        ctx->xref_table = NULL;
        return code;
    }

    if (pdfi_array_size(a) != 3) {
        pdfi_countdown(a);
        pdfi_close_file(ctx, XRefStrm);
        pdfi_countdown(ctx->xref_table);
        ctx->xref_table = NULL;
        return_error(gs_error_rangecheck);
    }
    for (i=0;i<3;i++) {
        // <MASK>
    }
    pdfi_countdown(a);

    code = pdfi_dict_get_type(ctx, sdict, "Index", PDF_ARRAY, (pdf_obj **)&a);
    if (code == gs_error_undefined) {
        code = read_xref_stream_entries(ctx, XRefStrm, 0, size - 1, W);
        if (code < 0) {
            pdfi_close_file(ctx, XRefStrm);
            pdfi_countdown(ctx->xref_table);
            ctx->xref_table = NULL;
            return code;
        }
    } else {
        int64_t start, size;

        if (code < 0) {
            pdfi_close_file(ctx, XRefStrm);
            pdfi_countdown(ctx->xref_table);
            ctx->xref_table = NULL;
            return code;
        }

        if (pdfi_array_size(a) & 1) {
            pdfi_countdown(a);
            pdfi_close_file(ctx, XRefStrm);
            pdfi_countdown(ctx->xref_table);
            ctx->xref_table = NULL;
            return_error(gs_error_rangecheck);
        }

        for (i=0;i < pdfi_array_size(a);i+=2){
            code = pdfi_array_get_int(ctx, a, (uint64_t)i, &start);
            if (code < 0 || start < 0) {
                pdfi_countdown(a);
                pdfi_close_file(ctx, XRefStrm);
                pdfi_countdown(ctx->xref_table);
                ctx->xref_table = NULL;
                return code;
            }

            code = pdfi_array_get_int(ctx, a, (uint64_t)i+1, &size);
            if (code < 0) {
                pdfi_countdown(a);
                pdfi_close_file(ctx, XRefStrm);
                pdfi_countdown(ctx->xref_table);
                ctx->xref_table = NULL;
                return code;
            }

            if (size < 1)
                continue;

            if (start + size >= ctx->xref_table->xref_size) {
                code = resize_xref(ctx, start + size);
                if (code < 0) {
                    pdfi_countdown(a);
                    pdfi_close_file(ctx, XRefStrm);
                    pdfi_countdown(ctx->xref_table);
                    ctx->xref_table = NULL;
                    return code;
                }
            }

            code = read_xref_stream_entries(ctx, XRefStrm, start, start + size - 1, W);
            if (code < 0) {
                pdfi_countdown(a);
                pdfi_close_file(ctx, XRefStrm);
                pdfi_countdown(ctx->xref_table);
                ctx->xref_table = NULL;
                return code;
            }
        }
    }
    pdfi_countdown(a);

    pdfi_close_file(ctx, XRefStrm);

    code = pdfi_dict_get_int(ctx, sdict, "Prev", &num);
    if (code == gs_error_undefined)
        return 0;

    if (code < 0)
        return code;

    if (num < 0 || num > ctx->main_stream_length)
        return_error(gs_error_rangecheck);

    if (pdfi_loop_detector_check_object(ctx, num) == true)
        return_error(gs_error_circular_reference);
    else {
        code = pdfi_loop_detector_add_object(ctx, num);
        if (code < 0)
            return code;
    }

    if(ctx->args.pdfdebug)
        dmprintf(ctx->memory, "%% Reading /Prev xref\n");

    pdfi_seek(ctx, s, num, SEEK_SET);

    code = pdfi_read_bare_int(ctx, ctx->main_stream, &objnum);
    if (code == 1) {
        if (pdfi_check_xref_stream(ctx))
            return pdfi_read_xref_stream_dict(ctx, s, objnum);
    }

    code = pdfi_read_bare_keyword(ctx, ctx->main_stream);
    if (code < 0)
        return code;
    if (code == TOKEN_XREF) {
        pdfi_set_error(ctx, 0, NULL, E_PDF_PREV_NOT_XREF_STREAM, "pdfi_process_xref_stream", NULL);
        if (!ctx->args.pdfstoponerror)
            /* Read old-style xref table */
            return(read_xref(ctx, ctx->main_stream));
    }
    return_error(gs_error_syntaxerror);
}