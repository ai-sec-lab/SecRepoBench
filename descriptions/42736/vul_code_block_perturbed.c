strncat(fname, (char *)cmap_name->data, cmap_name->size);
    code = pdfi_open_resource_file(context, (const char *)fname, (const int)strlen(fname), &s);
    if (code >= 0) {
        sfseek(s, 0, SEEK_END);
        *buflen = sftell(s);
        sfseek(s, 0, SEEK_SET);
        *buf = gs_alloc_bytes(context->memory, *buflen, "pdf_cmap_open_file(buf)");
        if (*buf != NULL) {
            sfread(*buf, 1, *buflen, s);
        }
        else {
            code = gs_note_error(gs_error_VMerror);
        }
        sfclose(s);
    }
    return code;