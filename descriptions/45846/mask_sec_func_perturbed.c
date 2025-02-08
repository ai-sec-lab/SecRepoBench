static int dfpwm_dec_frame(struct AVCodecContext *codeccontext, void *data,
    int *got_frame, struct AVPacket *packet)
{
    DFPWMState *state = codeccontext->priv_data;
    AVFrame *frame = data;
    int ret;

    // <MASK>
}