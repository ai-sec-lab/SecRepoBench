static int
pdf_cmap_open_file(pdf_context *ctx, gs_string *cmap_name, byte **buf, int64_t *buflen)
{
    int code = 0;
    stream *s;
    char fname[gp_file_name_sizeof];
    const char *path_pfx = "CMap/";
    fname[0] = '\0';

    strncat(fname, path_pfx, strlen(path_pfx));
    // <MASK>
}