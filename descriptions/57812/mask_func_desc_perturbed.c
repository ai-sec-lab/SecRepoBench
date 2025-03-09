static int pps_multilayer_extension(GetBitContext *gb, AVCodecContext *avctx,
                                    HEVCPPS *pictureparameterset, HEVCSPS *sps)
{
    pictureparameterset->poc_reset_info_present_flag = get_bits1(gb);
    pictureparameterset->pps_infer_scaling_list_flag = get_bits1(gb);
    if (pictureparameterset->pps_infer_scaling_list_flag)
        pictureparameterset->pps_scaling_list_ref_layer_id = get_bits(gb, 6);

    pictureparameterset->num_ref_loc_offsets = get_ue_golomb_long(gb);
    // Parse reference location offsets for multiple layers in the PPS
    // Iterate over all reference location offsets specified by num_ref_loc_offsets
    // For each offset, read and store the layer ID from the bitstream
    // Check and process flags for scaled reference layer offsets, reference region offsets, and resample phase set
    // Retrieve and store the respective offset values using Golomb-coded data as needed
    // Handle resample phases for luma and chroma components if specified
    // <MASK>

    pictureparameterset->colour_mapping_enabled_flag = get_bits1(gb);
    if (pictureparameterset->colour_mapping_enabled_flag)
        colour_mapping_table(gb, pictureparameterset);

    return 0;
}