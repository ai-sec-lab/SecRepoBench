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
        .second_chroma_qp_index_offset              = pps->chroma_qp_index_offset[1]