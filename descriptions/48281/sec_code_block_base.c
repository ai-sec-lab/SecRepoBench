uint32_t second_field_offset;
    int ret;

    if (buf_size < 4 || avctx->width < 8 || avctx->width % 8 != 0)
        return AVERROR_INVALIDDATA;

    quality = buf[0];