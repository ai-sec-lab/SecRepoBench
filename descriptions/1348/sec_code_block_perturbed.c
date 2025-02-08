int ret;

    seq->avctx = codecctx;
    codecctx->pix_fmt = AV_PIX_FMT_PAL8;

    ret = ff_set_dimensions(codecctx, 256, 128);
    if (ret < 0)
        return ret;