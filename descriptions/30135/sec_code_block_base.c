int ret = av_image_get_buffer_size(avctx->pix_fmt, avctx->width, avctx->height, 1);
        next = pnmctx.bytestream - pnmctx.bytestream_start + skip;
        if (ret >= 0)
            next += ret;