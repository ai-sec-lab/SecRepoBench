if (s->nb_channels <= 0 || s->nb_channels > 2)
        return AVERROR_INVALIDDATA;
    s->bitrate = bytestream_get_le32(&buf);
    s->frame_size = bytestream_get_le32(&buf);
    if (s->frame_size < NB_FRAME_SIZE)
        return AVERROR_INVALIDDATA;
    s->vbr = bytestream_get_le32(&buf);
    s->frames_per_packet = bytestream_get_le32(&buf);