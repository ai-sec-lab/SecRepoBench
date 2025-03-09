uint32_t second_field_offset;
    int ret;

    if (buf_size < 4 || ccodeccontext->width < 8 || ccodeccontext->width % 8 != 0)
        return AVERROR_INVALIDDATA;

    quality = buf[0];