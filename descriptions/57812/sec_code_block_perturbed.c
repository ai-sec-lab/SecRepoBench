if (pictureparameterset->num_ref_loc_offsets > FF_ARRAY_ELEMS(pictureparameterset->ref_loc_offset_layer_id)) {
        pictureparameterset->num_ref_loc_offsets = 0;
        return AVERROR_INVALIDDATA;
    }
    for (int i = 0; i < pictureparameterset->num_ref_loc_offsets; i++) {
        pictureparameterset->ref_loc_offset_layer_id[i] = get_bits(gb, 6);
        pictureparameterset->scaled_ref_layer_offset_present_flag[i] = get_bits1(gb);
        if (pictureparameterset->scaled_ref_layer_offset_present_flag[i]) {
            pictureparameterset->scaled_ref_layer_left_offset[pictureparameterset->ref_loc_offset_layer_id[i]]   = get_se_golomb_long(gb);
            pictureparameterset->scaled_ref_layer_top_offset[pictureparameterset->ref_loc_offset_layer_id[i]]    = get_se_golomb_long(gb);
            pictureparameterset->scaled_ref_layer_right_offset[pictureparameterset->ref_loc_offset_layer_id[i]]  = get_se_golomb_long(gb);
            pictureparameterset->scaled_ref_layer_bottom_offset[pictureparameterset->ref_loc_offset_layer_id[i]] = get_se_golomb_long(gb);
        }

        pictureparameterset->ref_region_offset_present_flag[i] = get_bits1(gb);
        if (pictureparameterset->ref_region_offset_present_flag[i]) {
            pictureparameterset->ref_region_left_offset[pictureparameterset->ref_loc_offset_layer_id[i]]   = get_se_golomb_long(gb);
            pictureparameterset->ref_region_top_offset[pictureparameterset->ref_loc_offset_layer_id[i]]    = get_se_golomb_long(gb);
            pictureparameterset->ref_region_right_offset[pictureparameterset->ref_loc_offset_layer_id[i]]  = get_se_golomb_long(gb);
            pictureparameterset->ref_region_bottom_offset[pictureparameterset->ref_loc_offset_layer_id[i]] = get_se_golomb_long(gb);
        }

        pictureparameterset->resample_phase_set_present_flag[i] = get_bits1(gb);
        if (pictureparameterset->resample_phase_set_present_flag[i]) {
            pictureparameterset->phase_hor_luma[pictureparameterset->ref_loc_offset_layer_id[i]]         = get_ue_golomb_long(gb);
            pictureparameterset->phase_ver_luma[pictureparameterset->ref_loc_offset_layer_id[i]]         = get_ue_golomb_long(gb);
            pictureparameterset->phase_hor_chroma_plus8[pictureparameterset->ref_loc_offset_layer_id[i]] = get_ue_golomb_long(gb);
            pictureparameterset->phase_ver_chroma_plus8[pictureparameterset->ref_loc_offset_layer_id[i]] = get_ue_golomb_long(gb);
        }
    }