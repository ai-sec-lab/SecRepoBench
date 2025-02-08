static int vp9_superframe_filter(AVBSFContext *ctx, AVPacket *pkt)
{
    GetBitContext gb;
    VP9BSFContext *s = ctx->priv_data;
    int res, isinvisible, profile, marker, uses_superframe_syntax = 0, n;

    res = ff_bsf_get_packet_ref(ctx, pkt);
    if (res < 0)
        return res;

    // <MASK>
}