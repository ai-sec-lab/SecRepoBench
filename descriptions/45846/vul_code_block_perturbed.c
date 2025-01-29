frame->nb_samples = packet->size * 8 / codeccontext->ch_layout.nb_channels;
    if (frame->nb_samples <= 0) {
        av_log(codeccontext, AV_LOG_ERROR, "invalid number of samples in packet\n");
        return AVERROR_INVALIDDATA;
    }

    if ((ret = ff_get_buffer(codeccontext, frame, 0)) < 0)
        return ret;

    au_decompress(state, 140, packet->size, frame->data[0], packet->data);

    *got_frame = 1;
    return packet->size;