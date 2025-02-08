int i, code = 0;

    for (i = 0; i < 27; i++) {
        if (memcmp(str, op_table_1[i], 1) == 0) {
            code = pdfi_object_alloc(pdfctx, PDF_KEYWORD, 1, (pdf_obj **)key);
            if (code < 0)
                return code;
            memcpy((*key)->data, str, 1);
            (*key)->key = TOKEN_NOT_A_KEYWORD;
            pdfi_countup(*key);
            return 1;
        }
    }
    return 0;