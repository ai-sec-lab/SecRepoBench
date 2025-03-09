if (max_matrix_channel > std_max_matrix_channel) {
        av_log(m->avctx, AV_LOG_ERROR,
               "Max matrix channel cannot be greater than %d.\n",
               std_max_matrix_channel);
        return AVERROR_INVALIDDATA;
    }

    /* This should happen for TrueHD streams with >6 channels and MLP's noise
     * type. It is not yet known if this is allowed. */
    if (maxaudiochannel > MAX_MATRIX_CHANNEL_MLP && !noise_type) {
        avpriv_request_sample(m->avctx,
                              "%d channels (more than the "
                              "maximum supported by the decoder)",
                              maxaudiochannel + 2);
        return AVERROR_PATCHWELCOME;
    }