static int pps_multilayer_extension(GetBitContext *gb, AVCodecContext *avctx,
                                    HEVCPPS *pictureparameterset, HEVCSPS *sps)
{
    pictureparameterset->poc_reset_info_present_flag = get_bits1(gb);
    pictureparameterset->pps_infer_scaling_list_flag = get_bits1(gb);
    if (pictureparameterset->pps_infer_scaling_list_flag)
        pictureparameterset->pps_scaling_list_ref_layer_id = get_bits(gb, 6);

    pictureparameterset->num_ref_loc_offsets = get_ue_golomb_long(gb);
    // <MASK>

    pictureparameterset->colour_mapping_enabled_flag = get_bits1(gb);
    if (pictureparameterset->colour_mapping_enabled_flag)
        colour_mapping_table(gb, pictureparameterset);

    return 0;
}