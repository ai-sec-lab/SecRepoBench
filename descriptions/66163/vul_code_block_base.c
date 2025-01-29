num_ref_idx = num_ref_idx_l0_active;

	gf_bs_read_ue_log(bs, "luma_log2_weight_denom");
	if (ChromaArrayType != 0)
		gf_bs_read_se_log(bs, "delta_chroma_log2_weight_denom");

parse_weights:
	for (i = 0; i < num_ref_idx; i++) {
		luma_weights[i] = gf_bs_read_int_log_idx(bs, 1, "luma_weights", i);
		//inferred to be 0 if not present
		chroma_weights[i] = 0;
	}
	if (ChromaArrayType != 0) {
		for (i = 0; i < num_ref_idx; i++) {
			chroma_weights[i] = gf_bs_read_int_log_idx(bs, 1, "chroma_weights", i);
		}
	}
	for (i = 0; i < num_ref_idx; i++) {
		if (luma_weights[i]) {
			gf_bs_read_se_log_idx(bs, "delta_luma_weight_l0", i);
			gf_bs_read_se_log_idx(bs, "luma_offset_l0", i);
		}
		if (chroma_weights[i]) {
			gf_bs_read_se_log_idx(bs, "delta_chroma_weight_l0_0", i);
			gf_bs_read_se_log_idx(bs, "delta_chroma_offset_l0_0", i);

			gf_bs_read_se_log_idx(bs, "delta_chroma_weight_l0_1", i);
			gf_bs_read_se_log_idx(bs, "delta_chroma_offset_l0_1", i);
		}
	}

	if (si->slice_type == GF_HEVC_SLICE_TYPE_B) {
		if (!first_pass) return;
		first_pass = GF_FALSE;
		num_ref_idx = num_ref_idx_l1_active;
		goto parse_weights;
	}