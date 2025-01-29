if (avctx->width & 15)
        return AVERROR_PATCHWELCOME;

    s->vectors = av_malloc((avctx->width * avctx->height * 3) / 2);
    if (!s->vectors)
        return AVERROR(ENOMEM);

    s->coeff = av_malloc_array(2 * avctx->width, sizeof(s->coeff[0]));
    if (!s->coeff)
        return AVERROR(ENOMEM);

    s->tmp1 = av_malloc_array(avctx->width / 2, sizeof(s->tmp1[0]));
    if (!s->tmp1)
        return AVERROR(ENOMEM);

    s->tmp2 = av_malloc_array(avctx->width / 2, sizeof(s->tmp2[0]));
    if (!s->tmp2)
        return AVERROR(ENOMEM);

    avctx->pix_fmt = AV_PIX_FMT_YUV420P;
    s->frame = av_frame_alloc();
    if (!s->frame)
        return AVERROR(ENOMEM);

    ff_thread_once(&initializeonceflag, vqc_init_static_data);

    return 0;