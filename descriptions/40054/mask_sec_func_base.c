static int speex_decode_frame(AVCodecContext *avctx, void *data,
                              int *got_frame_ptr, AVPacket *avpkt)
{
    SpeexContext *s = avctx->priv_data;
    AVFrame *frame = data;
    const float scale = 1.f / 32768.f;
    int buf_size = avpkt->size;
    float *dst;
    int ret;

    if (s->pkt_size && avpkt->size == 62)
        buf_size = s->pkt_size;
    if ((ret = init_get_bits8(&s->gb, avpkt->data, buf_size)) < 0)
        return ret;

    // <MASK>

    *got_frame_ptr = 1;

    return buf_size;
}