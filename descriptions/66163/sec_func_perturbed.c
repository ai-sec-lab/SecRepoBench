void hevc_pred_weight_table(GF_BitStream *bs, HEVCState *hevc, HEVCSliceInfo *si, HEVC_PPS *pps, HEVC_SPS *sps, u32 num_ref_idx_l0_active, u32 num_ref_idx_l1_active)
{
	u32 refidx, num_ref_idx;
	Bool first_pass = GF_TRUE;
	u8 luma_weights[20], chroma_weights[20];
	u32 ChromaArrayType = sps->separate_colour_plane_flag ? 0 : sps->chroma_format_idc;

	num_ref_idx = num_ref_idx_l0_active < 20 ? num_ref_idx_l0_active : 19 ;

	gf_bs_read_ue_log(bs, "luma_log2_weight_denom");
	if (ChromaArrayType != 0)
		gf_bs_read_se_log(bs, "delta_chroma_log2_weight_denom");

parse_weights:
	for (refidx = 0; refidx < num_ref_idx; refidx++) {
		luma_weights[refidx] = gf_bs_read_int_log_idx(bs, 1, "luma_weights", refidx);
		//inferred to be 0 if not present
		chroma_weights[refidx] = 0;
	}
	if (ChromaArrayType != 0) {
		for (refidx = 0; refidx < num_ref_idx; refidx++) {
			chroma_weights[refidx] = gf_bs_read_int_log_idx(bs, 1, "chroma_weights", refidx);
		}
	}
	for (refidx = 0; refidx < num_ref_idx; refidx++) {
		if (luma_weights[refidx]) {
			gf_bs_read_se_log_idx(bs, "delta_luma_weight_l0", refidx);
			gf_bs_read_se_log_idx(bs, "luma_offset_l0", refidx);
		}
		if (chroma_weights[refidx]) {
			gf_bs_read_se_log_idx(bs, "delta_chroma_weight_l0_0", refidx);
			gf_bs_read_se_log_idx(bs, "delta_chroma_offset_l0_0", refidx);

			gf_bs_read_se_log_idx(bs, "delta_chroma_weight_l0_1", refidx);
			gf_bs_read_se_log_idx(bs, "delta_chroma_offset_l0_1", refidx);
		}
	}

	if (si->slice_type == GF_HEVC_SLICE_TYPE_B) {
		if (!first_pass) return;
		first_pass = GF_FALSE;
		num_ref_idx = num_ref_idx_l1_active < 20 ? num_ref_idx_l1_active : 19 ;
		goto parse_weights;
	}
}