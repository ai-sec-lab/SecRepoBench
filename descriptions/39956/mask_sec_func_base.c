static int parse_speex_extradata(AVCodecContext *avctx,
    const uint8_t *extradata, int extradata_size)
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
    if (s->nb_channels <= 0 || s->nb_channels > 2)
        return AVERROR_INVALIDDATA;
    s->bitrate = bytestream_get_le32(&buf);
    s->frame_size = bytestream_get_le32(&buf);
    if (s->frame_size < NB_FRAME_SIZE)
        return AVERROR_INVALIDDATA;
    s->vbr = bytestream_get_le32(&buf);
    s->frames_per_packet = bytestream_get_le32(&buf);
    // <MASK>
}