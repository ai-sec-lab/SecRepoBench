if (nb_slices * 8LL + slices_info_offset > buf_size - 16 || slice_height <= 0)
            return AVERROR_INVALIDDATA;