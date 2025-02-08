static int pps_multilayer_extension(GetBitContext *gb, AVCodecContext *codecctx,
                                    HEVCPPS *pps, HEVCSPS *sps)
{
    pps->poc_reset_info_present_flag = get_bits1(gb);
    pps->pps_infer_scaling_list_flag = get_bits1(gb);
    if (pps->pps_infer_scaling_list_flag)
        pps->pps_scaling_list_ref_layer_id = get_bits(gb, 6);

    pps->num_ref_loc_offsets = get_ue_golomb_long(gb);
    // <MASK>

    pps->colour_mapping_enabled_flag = get_bits1(gb);
    if (pps->colour_mapping_enabled_flag)
        colour_mapping_table(gb, pps);

    return 0;
}