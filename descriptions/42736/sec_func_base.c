static int
pdf_cmap_open_file(pdf_context *ctx, gs_string *cmap_name, byte **buf, int64_t *buflen)
{
    int code = 0;
    stream *s;
    char fname[gp_file_name_sizeof];
    const char *path_pfx = "CMap/";
    fname[0] = '\0';

    strncat(fname, path_pfx, strlen(path_pfx));
    if (strlen(fname) + cmap_name->size >= gp_file_name_sizeof)
        return_error(gs_error_rangecheck);

    strncat(fname, (char *)cmap_name->data, cmap_name->size);
    code = pdfi_open_resource_file(ctx, (const char *)fname, (const int)strlen(fname), &s);
    if (code >= 0) {
        sfseek(s, 0, SEEK_END);
        *buflen = sftell(s);
        sfseek(s, 0, SEEK_SET);
        *buf = gs_alloc_bytes(ctx->memory, *buflen, "pdf_cmap_open_file(buf)");
        if (*buf != NULL) {
            sfread(*buf, 1, *buflen, s);
        }
        else {
            code = gs_note_error(gs_error_VMerror);
        }
        sfclose(s);
    }
    return code;
}