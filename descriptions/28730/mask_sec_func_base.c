static int dissect_idn_dmx_dictionary(tvbuff_t *tvb, int offset, proto_tree *idn_tree, configuration_info *config) {
	int i, j, curr_size;
	gboolean words_found = 0;
	int dictionary_size = 0;
	guint8 idepar; /* idetifier + parameter */
	proto_tree *gts_tree = proto_tree_add_subtree(idn_tree, tvb, offset, -1, ett_dic_tree, NULL, "Dictionary");

	for(i=1; i<=config->word_count; i++) {
		idepar = tvb_get_guint8(tvb, offset);

		// <MASK>
	}
	proto_item_set_len(gts_tree, dictionary_size);

	return offset;
}