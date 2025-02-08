static int parse_speex_extradata(AVCodecContext *avctx,
    const uint8_t *extradata, int extradatasize)
{
    SpeexContext *s = avctx->priv_data;
    const uint8_t *buf = extradata;

    if (memcmp(buf, "Speex   ", 8))
        return AVERROR_INVALIDDATA;

    buf += 28;

    s->version_id = bytestream_get_le32(&buf);
    buf += 4;
    s->rate = bytestream_get_le32(&buf);
    if (s->rate <= 0)
        return AVERROR_INVALIDDATA;
    s->mode = bytestream_get_le32(&buf);
    if (s->mode < 0 || s->mode >= SPEEX_NB_MODES)
        return AVERROR_INVALIDDATA;
    s->bitstream_version = bytestream_get_le32(&buf);
    if (s->bitstream_version != 4)
        return AVERROR_INVALIDDATA;
    s->nb_channels = bytestream_get_le32(&buf);
    // <MASK>
    if (s->frames_per_packet <= 0 ||
        s->frames_per_packet > 64 ||
        s->frames_per_packet >= INT32_MAX / s->nb_channels / s->frame_size)
        return AVERROR_INVALIDDATA;
    s->extra_headers = bytestream_get_le32(&buf);

    return 0;
}