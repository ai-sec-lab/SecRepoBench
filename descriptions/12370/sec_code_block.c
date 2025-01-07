if (nb_slices * 8LL + slices_info_offset > buf_size - 16 ||
            slice_height <= 0 || nb_slices * (uint64_t)slice_height > height)
            return AVERROR_INVALIDDATA;