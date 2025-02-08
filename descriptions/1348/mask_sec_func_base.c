static av_cold int seqvideo_decode_init(AVCodecContext *avctx)
{
    SeqVideoContext *seq = avctx->priv_data;
    // <MASK>

    seq->frame = av_frame_alloc();
    if (!seq->frame)
        return AVERROR(ENOMEM);

    return 0;
}