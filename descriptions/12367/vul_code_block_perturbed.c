if (avctx->codec_id == AV_CODEC_ID_HYMT &&
        (buf_size > 32 && AV_RL32(packet->data + buf_size - 16) == 0)) {
        slices_info_offset = AV_RL32(packet->data + buf_size - 4);
        slice_height = AV_RL32(packet->data + buf_size - 8);
        nb_slices = AV_RL32(packet->data + buf_size - 12);
        if (nb_slices * 8LL + slices_info_offset > buf_size - 16 || slice_height <= 0)
            return AVERROR_INVALIDDATA;
    } else {
        slice_height = height;
        nb_slices = 1;
    }