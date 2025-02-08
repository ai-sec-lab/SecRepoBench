if (cidname == NULL || cidname->type != PDF_NAME)
            goto exit;

        memcpy(fontfname, fsprefix, fsprefixlen);
        memcpy(fontfname + fsprefixlen, cidname->data, cidname->length);
        fontfname[fsprefixlen + cidname->length] = '\0';

        code = pdfi_open_resource_file(ctx, fontfname, strlen(fontfname), &s);
        if (code < 0) {
            code = gs_note_error(gs_error_invalidfont);
        }
        else {
            sfseek(s, 0, SEEK_END);
            *buflen = sftell(s);
            sfseek(s, 0, SEEK_SET);
            *buf = gs_alloc_bytes(ctx->memory, *buflen, "pdfi_open_CIDFont_file(buf)");
            if (*buf != NULL) {
                sfread(*buf, 1, *buflen, s);
            }
            else {
                code = gs_note_error(gs_error_invalidfont);
            }
            sfclose(s);
        }