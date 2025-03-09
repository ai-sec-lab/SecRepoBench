c->padded_bits  = aligned_width - c->avctx->width;
    if (c->video_size < aligned_width * codeccontext->height * (int64_t)c->bpp / 8)
        return AVERROR_INVALIDDATA;
    if (!encoding && c->palette_size && c->bpp <= 8) {
        codeccontext->pix_fmt = AV_PIX_FMT_PAL8;
    } else if (encoding == 1 && (c->bpp == 6 || c->bpp == 8)) {
        if (c->palette_size != (1 << (c->bpp - 1)))
            return AVERROR_INVALIDDATA;
        codeccontext->pix_fmt = AV_PIX_FMT_BGR24;
    } else if (!encoding && c->bpp == 24 && c->format == CHUNKY &&
               !c->palette_size) {
        codeccontext->pix_fmt = AV_PIX_FMT_RGB24;
    } else {
        avpriv_request_sample(codeccontext, "Encoding %d, bpp %d and format 0x%x",
                              encoding, c->bpp, c->format);
        return AVERROR_PATCHWELCOME;
    }