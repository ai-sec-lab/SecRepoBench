    av_assert0(avctx->ch_layout.nb_channels >= 0);
    if (avctx->ch_layout.nb_channels > WMALL_MAX_CHANNELS) {
        avpriv_request_sample(avctx,
                              "More than " AV_STRINGIFY(WMALL_MAX_CHANNELS) " channels");
        return AVERROR_PATCHWELCOME;
    }

    if (avctx->extradata_size >= 18) {
        s->decode_flags    = AV_RL16(edata_ptr + 14);
        channel_mask       = AV_RL32(edata_ptr +  2);
        s->bits_per_sample = AV_RL16(edata_ptr);
        if (s->bits_per_sample == 16)
            avctx->sample_fmt = AV_SAMPLE_FMT_S16P;
        else if (s->bits_per_sample == 24) {
            avctx->sample_fmt = AV_SAMPLE_FMT_S32P;
            avctx->bits_per_raw_sample = 24;
        } else {
            av_log(avctx, AV_LOG_ERROR, "Unknown bit-depth: %"PRIu8"\n",
                   s->bits_per_sample);
            return AVERROR_INVALIDDATA;
        }
        /* dump the extradata */
        for (i = 0; i < avctx->extradata_size; i++)
            ff_dlog(avctx, "[%x] ", avctx->extradata[i]);
        ff_dlog(avctx, "\n");

    } else {
        avpriv_request_sample(avctx, "Unsupported extradata size");
        return AVERROR_PATCHWELCOME;
    }

    if (channel_mask) {
        av_channel_layout_uninit(&avctx->ch_layout);
        av_channel_layout_from_mask(&avctx->ch_layout, channel_mask);
    }