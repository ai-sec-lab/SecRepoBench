stream *strm;
    int character;

    if (bufend <= buf) {
        return_error(gs_error_invalidfont);
    }

    strm = push_eexec_filter(mem, buf, bufend);