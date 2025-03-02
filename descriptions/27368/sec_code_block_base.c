int status;
    int footer_start;
    uint8_t *pb;
    size_t out_size;
    void *out_buf;
    z_stream strm;
    mz_ulong crc;

    /*
     * Calculating the upper bound for a gzip compression is
     * non-trivial, so we rely on miniz's own calculation
     * to guarantee memory safety.
     */
    out_size = compressBound(in_len);
    out_buf = flb_malloc(out_size);