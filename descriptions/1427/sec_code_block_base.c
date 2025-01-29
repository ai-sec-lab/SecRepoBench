if (c->video_size < aligned_width * avctx->height * (int64_t)c->bpp / 8)
        return AVERROR_INVALIDDATA;
    if (!encoding && c->palette_size && c->bpp <= 8 && c->format != CHUNKY) {
        avctx->pix_fmt = AV_PIX_FMT_PAL8;
    } else if (encoding == 1 && (c->bpp == 6 || c->bpp == 8) && c->format != CHUNKY) {
        if (c->palette_size != (1 << (c->bpp - 1)))
            return AVERROR_INVALIDDATA;
        avctx->pix_fmt = AV_PIX_FMT_BGR24;
    } else if (!encoding && c->bpp == 24 && c->format == CHUNKY &&
               !c->palette_size) {
        avctx->pix_fmt = AV_PIX_FMT_RGB24;
    } else {
        avpriv_request_sample(avctx, "Encoding %d, bpp %d and format 0x%x",
                              encoding, c->bpp, c->format);
        return AVERROR_PATCHWELCOME;
    }

    if ((ret = ff_get_buffer(avctx, p, 0)) < 0)
        return ret;
    p->pict_type = AV_PICTURE_TYPE_I;

    if (encoding) {
        av_fast_padded_malloc(&c->new_video, &c->new_video_size,
                              h * w + AV_INPUT_BUFFER_PADDING_SIZE);
        if (!c->new_video)
            return AVERROR(ENOMEM);
        if (c->bpp == 8)
            cdxl_decode_ham8(c, p);
        else
            cdxl_decode_ham6(c, p);
    } else if (avctx->pix_fmt == AV_PIX_FMT_PAL8) {
        cdxl_decode_rgb(c, p);
    } else {
        cdxl_decode_raw(c, p);
    }
    *got_frame = 1;

    return buf_size;