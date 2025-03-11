int status;
    int footer_start;
    uint8_t *pb;
    size_t out_size;
    void *out_buf;
    z_stream strm;
    mz_ulong crc;


    /*
     * GZIP relies on an algorithm with worst-case expansion
     * of 5 bytes per 32KB data. This means we need to create a variable
     * length output, that depends on the input length.
     * See RFC 1951 for details.
     */
    int max_input_expansion = ((int)(input_size / 32000) + 1) * 5;

    /*
     * Max compressed size is equal to sum of:
     *   10 byte header
     *   8 byte foot
     *   max input expansion
     *   size of input
     */
    out_size = 10 + 8 + max_input_expansion + input_size;
    out_buf = flb_malloc(out_size);