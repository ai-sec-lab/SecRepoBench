if (pps->num_ref_loc_offsets > FF_ARRAY_ELEMS(pps->ref_loc_offset_layer_id)) {
        pps->num_ref_loc_offsets = 0;
        return AVERROR_INVALIDDATA;
    }
    for (int i = 0; i < pps->num_ref_loc_offsets; i++) {
        pps->ref_loc_offset_layer_id[i] = get_bits(gb, 6);
        pps->scaled_ref_layer_offset_present_flag[i] = get_bits1(gb);
        if (pps->scaled_ref_layer_offset_present_flag[i]) {
            pps->scaled_ref_layer_left_offset[pps->ref_loc_offset_layer_id[i]]   = get_se_golomb_long(gb);
            pps->scaled_ref_layer_top_offset[pps->ref_loc_offset_layer_id[i]]    = get_se_golomb_long(gb);
            pps->scaled_ref_layer_right_offset[pps->ref_loc_offset_layer_id[i]]  = get_se_golomb_long(gb);
            pps->scaled_ref_layer_bottom_offset[pps->ref_loc_offset_layer_id[i]] = get_se_golomb_long(gb);
        }

        pps->ref_region_offset_present_flag[i] = get_bits1(gb);
        if (pps->ref_region_offset_present_flag[i]) {
            pps->ref_region_left_offset[pps->ref_loc_offset_layer_id[i]]   = get_se_golomb_long(gb);
            pps->ref_region_top_offset[pps->ref_loc_offset_layer_id[i]]    = get_se_golomb_long(gb);
            pps->ref_region_right_offset[pps->ref_loc_offset_layer_id[i]]  = get_se_golomb_long(gb);
            pps->ref_region_bottom_offset[pps->ref_loc_offset_layer_id[i]] = get_se_golomb_long(gb);
        }

        pps->resample_phase_set_present_flag[i] = get_bits1(gb);
        if (pps->resample_phase_set_present_flag[i]) {
            pps->phase_hor_luma[pps->ref_loc_offset_layer_id[i]]         = get_ue_golomb_long(gb);
            pps->phase_ver_luma[pps->ref_loc_offset_layer_id[i]]         = get_ue_golomb_long(gb);
            pps->phase_hor_chroma_plus8[pps->ref_loc_offset_layer_id[i]] = get_ue_golomb_long(gb);
            pps->phase_ver_chroma_plus8[pps->ref_loc_offset_layer_id[i]] = get_ue_golomb_long(gb);
        }
    }