static int jpegxl_collect_codestream_header(const uint8_t *input_buffer, int input_len,
                                            uint8_t *buffer, int buflen, int *copied) {
    GetByteContext gb;
    *copied = 0;
    bytestream2_init(&gb, input_buffer, input_len);

    while (1) {
        uint64_t size;
        uint32_t tag;
        int head_size = 8;

        if (bytestream2_get_bytes_left(&gb) < 16)
            break;

        size = bytestream2_get_be32(&gb);
        if (size == 1) {
            size = bytestream2_get_be64(&gb);
            head_size = 16;
        }
        /* invalid ISOBMFF size */
        if (size && size <= head_size)
            return AVERROR_INVALIDDATA;
        if (size)
            size -= head_size;

        tag = bytestream2_get_le32(&gb);
        if (tag == MKTAG('j', 'x', 'l', 'p')) {
            if (bytestream2_get_bytes_left(&gb) < 4)
                break;
            bytestream2_skip(&gb, 4);
            if (size) {
                if (size <= 4)
                    return AVERROR_INVALIDDATA;
                size -= 4;
            }
        }
        /*
         * size = 0 means "until EOF". this is legal but uncommon
         * here we just set it to the remaining size of the probe buffer
         */
        if (!size)
            size = bytestream2_get_bytes_left(&gb);

        if (tag == MKTAG('j', 'x', 'l', 'c') || tag == MKTAG('j', 'x', 'l', 'p')) {
            if (size > buflen - *copied)
                size = buflen - *copied;
            /*
             * arbitrary chunking of the payload makes this memcpy hard to avoid
             * in practice this will only be performed one or two times at most
             */
            *copied += bytestream2_get_buffer(&gb, buffer + *copied, size);
        } else {
            bytestream2_skip(&gb, size);
        }
        if (bytestream2_get_bytes_left(&gb) <= 0 || *copied >= buflen)
            break;
    }

    return bytestream2_tell(&gb);
}