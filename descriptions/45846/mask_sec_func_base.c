static int dfpwm_dec_frame(struct AVCodecContext *ctx, void *data,
    int *got_frame, struct AVPacket *packet)
{
    DFPWMState *state = ctx->priv_data;
    AVFrame *frame = data;
    int ret;

    // <MASK>
}