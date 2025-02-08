if (s->frames_per_packet <= 0 ||
        s->frames_per_packet > 64 ||
        s->frames_per_packet >= INT32_MAX / s->nb_channels / s->frame_size)
        return AVERROR_INVALIDDATA;
    s->extra_headers = bytestream_get_le32(&buf);

    return 0;