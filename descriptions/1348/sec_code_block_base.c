int ret;

    seq->avctx = avctx;
    avctx->pix_fmt = AV_PIX_FMT_PAL8;

    ret = ff_set_dimensions(avctx, 256, 128);
    if (ret < 0)
        return ret;