int code = 0;
    while (*ps != destination) {
        stream *s = *ps;
        gs_memory_t *mem = s->state->memory;
        gs_memory_t *cbuf_string_memory = s->cbuf_string_memory;
        byte *sbuf = s->cbuf;
        byte *cbuf = s->cbuf_string.data;
        stream *next = s->strm;
        int status = sclose(s);
        stream_state *ss = s->state; /* sclose may set this to s */

        if (code == 0)
            code = status;

        if (s->cbuf_string_memory != NULL) { /* stream owns string buffer, so free it */
            gs_free_object(cbuf_string_memory, cbuf, "s_close_filters(cbuf)");
        }

        if (mem) {
            if (sbuf != cbuf)
                gs_free_object(mem, sbuf, "s_close_filters(buf)");
            gs_free_object(mem, s, "s_close_filters(stream)");
            if (ss != (stream_state *)s)
                gs_free_object(mem, ss, "s_close_filters(state)");
        }
        *ps = next;
    }
    return code;