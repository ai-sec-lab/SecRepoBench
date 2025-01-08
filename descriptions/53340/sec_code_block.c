if (max_matrix_channel > MAX_MATRIX_CHANNEL_MLP && !noise_type) {
        avpriv_request_sample(m->avctx,
                              "%d channels (more than the "
                              "maximum supported by the decoder)",
                              max_channel + 2);
        return AVERROR_PATCHWELCOME;
    }