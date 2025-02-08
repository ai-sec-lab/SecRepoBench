static int vaapi_h264_start_frame(AVCodecContext          *avctx,
                                  av_unused const uint8_t *buffer,
                                  av_unused uint32_t       size)
{
    const H264Context *h = avctx->priv_data;
    VAAPIDecodePicture *pic = h->cur_pic_ptr->hwaccel_picture_private;
    const PPS *pps = h->ps.pps;
    const SPS *sps = h->ps.sps;
    VAPictureParameterBufferH264 pic_param;
    VAIQMatrixBufferH264 iq_matrix;
    int err;

    pic->output_surface = ff_vaapi_get_surface_id(h->cur_pic_ptr->f);

    pic_param = (VAPictureParameterBufferH264) {
        .picture_width_in_mbs_minus1                = h->mb_width - 1,
        .picture_height_in_mbs_minus1               = h->mb_height - 1,
        .bit_depth_luma_minus8                      = sps->bit_depth_luma - 8,
        .bit_depth_chroma_minus8                    = sps->bit_depth_chroma - 8,
        .num_ref_frames                             = sps->ref_frame_count,
        .seq_fields.bits = {
            .chroma_format_idc                      = sps->chroma_format_idc,
            .residual_colour_transform_flag         = sps->residual_color_transform_flag,
            .gaps_in_frame_num_value_allowed_flag   = sps->gaps_in_frame_num_allowed_flag,
            .frame_mbs_only_flag                    = sps->frame_mbs_only_flag,
            .mb_adaptive_frame_field_flag           = sps->mb_aff,
            .direct_8x8_inference_flag              = sps->direct_8x8_inference_flag,
            .MinLumaBiPredSize8x8                   = sps->level_idc >= 31, /* A.3.3.2 */
            .log2_max_frame_num_minus4              = sps->log2_max_frame_num - 4,
            .pic_order_cnt_type                     = sps->poc_type,
            .log2_max_pic_order_cnt_lsb_minus4      = sps->log2_max_poc_lsb - 4,
            .delta_pic_order_always_zero_flag       = sps->delta_pic_order_always_zero_flag,
        },
        .pic_init_qp_minus26                        = pps->init_qp - 26,
        .pic_init_qs_minus26                        = pps->init_qs - 26,
        .chroma_qp_index_offset                     = pps->chroma_qp_index_offset[0],
        .second_chroma_qp_index_offset              = pps->chroma_qp_index_offset[1],
        .pic_fields.bits = {
            .entropy_coding_mode_flag               = pps->cabac,
            .weighted_pred_flag                     = pps->weighted_pred,
            .weighted_bipred_idc                    = pps->weighted_bipred_idc,
            .transform_8x8_mode_flag                = pps->transform_8x8_mode,
            .field_pic_flag                         = h->picture_structure != PICT_FRAME,
            .constrained_intra_pred_flag            = pps->constrained_intra_pred,
            .pic_order_present_flag                 = pps->pic_order_present,
            .deblocking_filter_control_present_flag = pps->deblocking_filter_parameters_present,
            .redundant_pic_cnt_present_flag         = pps->redundant_pic_cnt_present,
            .reference_pic_flag                     = h->nal_ref_idc != 0,
        },
        .frame_num                                  = h->poc.frame_num,
    };

    fill_vaapi_pic(&pic_param.CurrPic, h->cur_pic_ptr, h->picture_structure);
    err = fill_vaapi_ReferenceFrames(&pic_param, h);
    if (err < 0)
        goto fail;

    err = ff_vaapi_decode_make_param_buffer(avctx, pic,
                                            VAPictureParameterBufferType,
                                            &pic_param, sizeof(pic_param));
    if (err < 0)
        goto fail;

    memcpy(iq_matrix.ScalingList4x4,
           pps->scaling_matrix4, sizeof(iq_matrix.ScalingList4x4));
    memcpy(iq_matrix.ScalingList8x8[0],
           pps->scaling_matrix8[0], sizeof(iq_matrix.ScalingList8x8[0]));
    memcpy(iq_matrix.ScalingList8x8[1],
           pps->scaling_matrix8[3], sizeof(iq_matrix.ScalingList8x8[0]));

    err = ff_vaapi_decode_make_param_buffer(avctx, pic,
                                            VAIQMatrixBufferType,
                                            &iq_matrix, sizeof(iq_matrix));
    if (err < 0)
        goto fail;

    return 0;

fail:
    ff_vaapi_decode_cancel(avctx, pic);
    return err;
}