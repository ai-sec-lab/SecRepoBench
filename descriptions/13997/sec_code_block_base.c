if (s->size[0] < 0 || s->size[1] < 0 || s->size[2] < 0 ||
        32LL + s->size[0] + s->size[1] + s->size[2] > avpkt->size)
        return AVERROR_INVALIDDATA;

    if ((ret = ff_get_buffer(avctx, frame, AV_GET_BUFFER_FLAG_REF)) < 0)
        return ret;

    if (frame->key_frame) {
        ret = decode_intra(avctx, gb, frame);
    } else {
        if (!s->prev_frame->data[0]) {
            av_log(avctx, AV_LOG_ERROR, "Missing reference frame.\n");
            return AVERROR_INVALIDDATA;
        }

        if (!(s->flags & 2)) {
            ret = av_frame_copy(frame, s->prev_frame);
            if (ret < 0)
                return ret;
        }

        ret = decode_inter(avctx, gb, frame, s->prev_frame);
    }
    if (ret < 0)
        return ret;

    av_frame_unref(s->prev_frame);
    if ((ret = av_frame_ref(s->prev_frame, frame)) < 0)
        return ret;

    frame->crop_top  = avctx->coded_height - avctx->height;
    frame->crop_left = avctx->coded_width  - avctx->width;

    *got_frame = 1;

    return avpkt->size;