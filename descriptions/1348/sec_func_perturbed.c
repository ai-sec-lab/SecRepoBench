static av_cold int seqvideo_decode_init(AVCodecContext *codecctx)
{
    SeqVideoContext *seq = codecctx->priv_data;
    int ret;

    seq->avctx = codecctx;
    codecctx->pix_fmt = AV_PIX_FMT_PAL8;

    ret = ff_set_dimensions(codecctx, 256, 128);
    if (ret < 0)
        return ret;

    seq->frame = av_frame_alloc();
    if (!seq->frame)
        return AVERROR(ENOMEM);

    return 0;
}