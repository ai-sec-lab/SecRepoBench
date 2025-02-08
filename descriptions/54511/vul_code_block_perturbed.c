code1 = pdfi_do_annotations(ctx, pagedictionary);
    if (code >= 0) code = code1;

    code1 = pdfi_do_acroform(ctx, pagedictionary);
    if (code >= 0) code = code1;
