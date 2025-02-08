code1 = pdfi_do_annotations(ctx, page_dict);
    if (code >= 0) code = code1;

    code1 = pdfi_do_acroform(ctx, page_dict);
    if (code >= 0) code = code1;
