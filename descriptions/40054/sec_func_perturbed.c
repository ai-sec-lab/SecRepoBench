static int speex_decode_frame(AVCodecContext *avctx, void *framedata,
                              int *got_frame_ptr, AVPacket *avpkt)
{
    SpeexContext *s = avctx->priv_data;
    AVFrame *frame = framedata;
    const float scale = 1.f / 32768.f;
    int buf_size = avpkt->size;
    float *dst;
    int ret;

    if (s->pkt_size && avpkt->size == 62)
        buf_size = s->pkt_size;
    if ((ret = init_get_bits8(&s->gb, avpkt->data, buf_size)) < 0)
        return ret;

    frame->nb_samples = FFALIGN(s->frame_size * s->frames_per_packet, 4);
    if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
        return ret;

    dst = (float *)frame->extended_data[0];
    for (int i = 0; i < s->frames_per_packet; i++) {
        ret = speex_modes[s->mode].decode(avctx, &s->st[s->mode], &s->gb, dst + i * s->frame_size);
        if (ret < 0)
            return ret;
        if (avctx->channels == 2)
            speex_decode_stereo(dst + i * s->frame_size, s->frame_size, &s->stereo);
    }

    dst = (float *)frame->extended_data[0];
    s->fdsp->vector_fmul_scalar(dst, dst, scale, frame->nb_samples * frame->channels);
    frame->nb_samples = s->frame_size * s->frames_per_packet;

    *got_frame_ptr = 1;

    return buf_size;
}