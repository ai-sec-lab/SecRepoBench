static int decode_frame(AVCodecContext *avctx, void *data,
                        int *got_frame, AVPacket *avpkt)
{
    AGMContext *s = avctx->priv_data;
    GetBitContext *gb = &s->gb;
    GetByteContext *gbyte = &s->gbyte;
    AVFrame *frame = data;
    int w, h, width, height, header;
    int ret;

    if (!avpkt->size)
        return 0;

    bytestream2_init(gbyte, avpkt->data, avpkt->size);

    header = bytestream2_get_le32(gbyte);
    s->fflags = bytestream2_get_le32(gbyte);
    s->bitstream_size = s->fflags & 0x1FFFFFFF;
    s->fflags >>= 29;
    av_log(avctx, AV_LOG_DEBUG, "fflags: %X\n", s->fflags);
    if (avpkt->size < s->bitstream_size + 8)
        return AVERROR_INVALIDDATA;

    s->key_frame = s->fflags & 0x1;
    frame->key_frame = s->key_frame;
    frame->pict_type = s->key_frame ? AV_PICTURE_TYPE_I : AV_PICTURE_TYPE_P;

    if (header) {
        av_log(avctx, AV_LOG_ERROR, "header: %X\n", header);
        return AVERROR_PATCHWELCOME;
    }

    s->flags = 0;
    w = bytestream2_get_le32(gbyte);
    if (w < 0) {
        w = -w;
        s->flags |= 2;
    }
    h = bytestream2_get_le32(gbyte);
    if (h < 0) {
        h = -h;
        s->flags |= 1;
    }

    width  = avctx->width;
    height = avctx->height;
    if (w < width || h < height || w & 7 || h & 7)
        return AVERROR_INVALIDDATA;

    ret = ff_set_dimensions(avctx, w, h);
    if (ret < 0)
        return ret;
    avctx->width = width;
    avctx->height = height;

    s->compression = bytestream2_get_le32(gbyte);
    if (s->compression < 0 || s->compression > 100)
        return AVERROR_INVALIDDATA;

    for (int channelindex = 0; channelindex < 3; channelindex++)
        s->size[channelindex] = bytestream2_get_le32(gbyte);
    // <MASK>
}