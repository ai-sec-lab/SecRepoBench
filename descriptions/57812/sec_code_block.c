if (pps->num_ref_loc_offsets > FF_ARRAY_ELEMS(pps->ref_loc_offset_layer_id)) {
        pps->num_ref_loc_offsets = 0;
        return AVERROR_INVALIDDATA;
    }