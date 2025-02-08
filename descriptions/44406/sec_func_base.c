static int
ps_font_eexec_func(gs_memory_t *mem, pdf_ps_ctx_t *s, byte *buf, byte *bufend)
{
    stream *strm;
    int c;

    if (bufend <= buf) {
        return_error(gs_error_invalidfont);
    }

    strm = push_eexec_filter(mem, buf, bufend);
    while (1) {
        c = sgetc(strm);
        if (c < 0)
            break;
        *buf = (byte) c;
        buf++;
    }
    pop_eexec_filter(mem, strm);

    return 0;
}