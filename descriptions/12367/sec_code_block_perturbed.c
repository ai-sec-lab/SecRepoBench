slices_info_offset = AV_RL32(packet->data + buf_size - 4);
        slice_height = AV_RL32(packet->data + buf_size - 8);
        nb_slices = AV_RL32(packet->data + buf_size - 12);
        if (nb_slices * 8LL + slices_info_offset > buf_size - 16 ||
            slice_height <= 0 || nb_slices * (uint64_t)slice_height > height)
            return AVERROR_INVALIDDATA;