if (c->video_size < aligned_width * avctx->height * (int64_t)c->bpp / 8)
        return AVERROR_INVALIDDATA;
    if (!compressiontype && c->palette_size && c->bpp <= 8 && c->format != CHUNKY) {
        avctx->pix_fmt = AV_PIX_FMT_PAL8;
    } else if (compressiontype == 1 && (c->bpp == 6 || c->bpp == 8) && c->format != CHUNKY) {
        if (c->palette_size != (1 << (c->bpp - 1)))
            return AVERROR_INVALIDDATA;
        avctx->pix_fmt = AV_PIX_FMT_BGR24;
    } else if (!compressiontype && c->bpp == 24 && c->format == CHUNKY &&
               !c->palette_size) {
        avctx->pix_fmt = AV_PIX_FMT_RGB24;
    } else {
        avpriv_request_sample(avctx, "Encoding %d, bpp %d and format 0x%x",
                              compressiontype, c->bpp, c->format);
        return AVERROR_PATCHWELCOME;
    }