static int cdxl_decode_frame(AVCodecContext *avctx, void *data,
                             int *got_frame, AVPacket *pkt)
{
    CDXLVideoContext *c = avctx->priv_data;
    AVFrame * const p = data;
    int ret, w, h, encoding, aligned_width, buf_size = pkt->size;
    const uint8_t *buf = pkt->data;

    if (buf_size < 32)
        return AVERROR_INVALIDDATA;
    encoding        = buf[1] & 7;
    c->format       = buf[1] & 0xE0;
    w               = AV_RB16(&buf[14]);
    h               = AV_RB16(&buf[16]);
    c->bpp          = buf[19];
    c->palette_size = AV_RB16(&buf[20]);
    c->palette      = buf + 32;
    c->video        = c->palette + c->palette_size;
    c->video_size   = buf_size - c->palette_size - 32;

    if (c->palette_size > 512)
        return AVERROR_INVALIDDATA;
    if (buf_size < c->palette_size + 32)
        return AVERROR_INVALIDDATA;
    if (c->bpp < 1)
        return AVERROR_INVALIDDATA;
    if (c->format != BIT_PLANAR && c->format != BIT_LINE && c->format != CHUNKY) {
        avpriv_request_sample(avctx, "Pixel format 0x%0x", c->format);
        return AVERROR_PATCHWELCOME;
    }

    if ((ret = ff_set_dimensions(avctx, w, h)) < 0)
        return ret;

    if (c->format == CHUNKY)
        aligned_width = avctx->width;
    else
        aligned_width = FFALIGN(c->avctx->width, 16);
    c->padded_bits  = aligned_width - c->avctx->width;
    // Check if the video size is sufficient based on the aligned width, height,
    // and bits per pixel (bpp). Return an error if it is not.
    // Determine the appropriate pixel format for the AVCodecContext based on
    // encoding, palette size, bits per pixel, and video format. If the video is
    // unencoded, has a palette, and uses <= 8 bpp, set the pixel format to PAL8.
    // If the video is encoded with bpp of 6 or 8 and the palette size matches,
    // set the pixel format to BGR24. If the video is unencoded, uses 24 bpp, is
    // in CHUNKY format, and has no palette, set the pixel format to RGB24.
    // If the format is unsupported, request a sample and return a patch-welcome
    // error to indicate the need for further support.
    // <MASK>

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
}