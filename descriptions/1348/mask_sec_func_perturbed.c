static av_cold int seqvideo_decode_init(AVCodecContext *codecctx)
{
    SeqVideoContext *seq = codecctx->priv_data;
    // <MASK>

    seq->frame = av_frame_alloc();
    if (!seq->frame)
        return AVERROR(ENOMEM);

    return 0;
}