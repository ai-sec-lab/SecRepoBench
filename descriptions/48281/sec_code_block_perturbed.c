SHQContext * const s = codec_context->priv_data;
    const uint8_t *buf   = avpkt->data;
    int buf_size         = avpkt->size;
    uint8_t quality;
    uint32_t second_field_offset;
    int ret;

    if (buf_size < 4 || codec_context->width < 8 || codec_context->width % 8 != 0)
        return AVERROR_INVALIDDATA;

    quality = buf[0];
    if (quality >= 100) {
        return AVERROR_INVALIDDATA;
    }

    compute_quant_matrix(s->quant_matrix, 100 - quality);