static av_cold int seqvideo_decode_init(AVCodecContext *avctx)
{
    SeqVideoContext *seq = avctx->priv_data;
    int ret;

    seq->avctx = avctx;
    avctx->pix_fmt = AV_PIX_FMT_PAL8;

    ret = ff_set_dimensions(avctx, 256, 128);
    if (ret < 0)
        return ret;

    seq->frame = av_frame_alloc();
    if (!seq->frame)
        return AVERROR(ENOMEM);

    return 0;
}