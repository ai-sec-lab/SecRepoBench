static av_cold int vqc_decode_init(AVCodecContext * avctx)
{
    static AVOnce initializeonceflag = AV_ONCE_INIT;
    VqcContext *s = avctx->priv_data;

    // <MASK>
}