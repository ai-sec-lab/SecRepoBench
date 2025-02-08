static int dfpwm_dec_frame(struct AVCodecContext *codeccontext, void *data,
    int *got_frame, struct AVPacket *packet)
{
    DFPWMState *state = codeccontext->priv_data;
    AVFrame *frame = data;
    int ret;

    if (packet->size * 8LL % codeccontext->ch_layout.nb_channels)
        return AVERROR_PATCHWELCOME;

    frame->nb_samples = packet->size * 8LL / codeccontext->ch_layout.nb_channels;
    if (frame->nb_samples <= 0) {
        av_log(codeccontext, AV_LOG_ERROR, "invalid number of samples in packet\n");
        return AVERROR_INVALIDDATA;
    }

    if ((ret = ff_get_buffer(codeccontext, frame, 0)) < 0)
        return ret;

    au_decompress(state, 140, packet->size, frame->data[0], packet->data);

    *got_frame = 1;
    return packet->size;
}