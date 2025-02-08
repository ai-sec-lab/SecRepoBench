static int dvbsub_parse_clut_segment(AVCodecContext *avctx,
                                     const uint8_t *buf, int buf_size)
{
    DVBSubContext *ctx = avctx->priv_data;

    const uint8_t *buf_end = buf + buf_size;
    int i, clut_id;
    int version;
    DVBSubCLUT *clut;
    int entry_id, depth , full_range;
    int y, cr, cb, alpha;
    int red, g, b, r_add, g_add, b_add;

    ff_dlog(avctx, "DVB clut packet:\n");

    for (i=0; i < buf_size; i++) {
        ff_dlog(avctx, "%02x ", buf[i]);
        if (i % 16 == 15)
            ff_dlog(avctx, "\n");
    }

    if (i % 16)
        ff_dlog(avctx, "\n");

    clut_id = *buf++;
    version = ((*buf)>>4)&15;
    buf += 1;

    clut = get_clut(ctx, clut_id);

    if (!clut) {
        clut = av_malloc(sizeof(DVBSubCLUT));
        if (!clut)
            return AVERROR(ENOMEM);

        memcpy(clut, &default_clut, sizeof(DVBSubCLUT));

        clut->id = clut_id;
        clut->version = -1;

        clut->next = ctx->clut_list;
        ctx->clut_list = clut;
    }

    if (clut->version != version) {

    clut->version = version;

    while (buf + 4 < buf_end) {
        // <MASK>
    }
    }

    return 0;
}