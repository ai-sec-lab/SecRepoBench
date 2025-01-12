    /* This should happen for TrueHD streams with >6 channels and MLP's noise
     * type. It is not yet known if this is allowed. */
    if (max_matrix_channel > MAX_MATRIX_CHANNEL_MLP && !noise_type) {
        avpriv_request_sample(m->avctx,
                              "%d channels (more than the "
                              "maximum supported by the decoder)",
                              max_channel + 2);
        return AVERROR_PATCHWELCOME;
    }