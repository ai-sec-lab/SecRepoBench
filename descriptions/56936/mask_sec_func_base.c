static int
dissect_gsmrlp(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void* data _U_)
{
	int reported_len = tvb_reported_length(tvb);
	proto_tree *rlp_tree;
	proto_item *ti;
	guint8 n_s, n_r;

	/* we currently support the 16bit header of RLP v0 + v1 */

	col_set_str(pinfo->cinfo, COL_PROTOCOL, "GSM-RLP");

	n_s = (tvb_get_guint8(tvb, 0)) >> 3 | ((tvb_get_guint8(tvb, 1) & 1) << 5);
	n_r = (tvb_get_guint8(tvb, 1) >> 2);

	ti = proto_tree_add_protocol_format(tree, proto_gsmrlp, tvb, 0, reported_len,
					    "GSM RLP");
	rlp_tree = proto_item_add_subtree(ti, ett_gsmrlp);

	proto_tree_add_item(rlp_tree, hf_gsmrlp_cr, tvb, 0, 1, ENC_BIG_ENDIAN);
	proto_tree_add_item(rlp_tree, hf_gsmrlp_pf, tvb, 1, 1, ENC_BIG_ENDIAN);
	// <MASK>

	return tvb_reported_length(tvb);
}