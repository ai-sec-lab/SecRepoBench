void *out_buf;
    z_stream strm;
    mz_ulong crc;

    out_size = in_len + 32;
    out_buf = flb_malloc(out_size);
    if (!out_buf) {
        flb_errno();
        flb_error("[gzip] could not allocate outgoing buffer");
        return -1;
    }