if (n_s == 0x3f) { /* U frame */
		guint u_ftype;
		proto_tree_add_uint(rlp_tree, hf_gsmrlp_ftype, tvb, 0, 1, RLP_FT_U);
		proto_tree_add_item_ret_uint(rlp_tree, hf_gsmrlp_u_ftype, tvb, 1, 1, ENC_BIG_ENDIAN, &u_ftype);
		if ((n_r & 0x1f) == RLP_U_FT_XID)
			dissect_gsmrlp_xid(tvb, 2, pinfo, rlp_tree);
		proto_item_append_text(ti, " U-Frame: %s", val_to_str(u_ftype, rlp_ftype_u_vals, "Unknown 0x%02x"));
	} else if (n_s == 0x3e) { /* S Frame */
		guint s_ftype;
		proto_tree_add_uint(rlp_tree, hf_gsmrlp_ftype, tvb, 0, 1, RLP_FT_S);
		proto_tree_add_item_ret_uint(rlp_tree, hf_gsmrlp_s_ftype, tvb, 0, 1, ENC_BIG_ENDIAN, &s_ftype);
		proto_tree_add_uint(rlp_tree, hf_gsmrlp_n_r, tvb, 1, 1, n_r);
		proto_item_append_text(ti, " S-Frame: %s, N(S): %u, N(R): %u",
				       val_to_str(s_ftype, rlp_ftype_s_vals, "Unknown 0x%02x"), n_s, n_r);
	} else { /* IS Frame */
		tvbuff_t *next_tvb;
		guint s_ftype;
		int data_len;

		proto_tree_add_uint(rlp_tree, hf_gsmrlp_ftype, tvb, 0, 1, RLP_FT_IS);
		proto_tree_add_item_ret_uint(rlp_tree, hf_gsmrlp_s_ftype, tvb, 0, 1, ENC_BIG_ENDIAN, &s_ftype);
		proto_tree_add_uint(rlp_tree, hf_gsmrlp_n_s, tvb, 0, 2, n_s);
		proto_tree_add_uint(rlp_tree, hf_gsmrlp_n_r, tvb, 1, 1, n_r);
		proto_item_append_text(ti, " IS-Frame: %s, N(S): %u, N(R): %u",
				       val_to_str(s_ftype, rlp_ftype_s_vals, "Unknown 0x%02x"), n_s, n_r);

		/* dispatch user data */
		data_len = reportedlength - 2 /* header */ - 3 /* FCS */;
		next_tvb = tvb_new_subset_length(tvb, 2, data_len);
		if (decode_as_l2rcop && l2rcop_handle)
			call_dissector(l2rcop_handle, next_tvb, pinfo, rlp_tree);
		else
			call_data_dissector(next_tvb, pinfo, rlp_tree);
	}

	/* FCS is always the last 3 bytes of the message */
	guint32 fcs_computed = rlp_fcs_compute(tvb_get_ptr(tvb, 0, reportedlength - 3), reportedlength - 3);
	proto_tree_add_checksum(rlp_tree, tvb, reportedlength - 3, hf_gsmrlp_fcs, hf_gsmrlp_fcs_status,
				&ei_gsmrlp_fcs_bad, pinfo, fcs_computed, ENC_BIG_ENDIAN, PROTO_CHECKSUM_VERIFY);