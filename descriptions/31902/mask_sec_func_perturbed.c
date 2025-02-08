GF_Err abst_box_read(GF_Box *Thenewvariablenameforscouldbebox, GF_BitStream *bs)
{
	GF_AdobeBootstrapInfoBox *ptr = (GF_AdobeBootstrapInfoBox *)Thenewvariablenameforscouldbebox;
	int i;
	u32 tmp_strsize;
	char *tmp_str;
	// <MASK>
	if (i) {
		ptr->meta_data = gf_strdup(tmp_str);
	}

	ISOM_DECREASE_SIZE(ptr, 1)
	ptr->segment_run_table_count = gf_bs_read_u8(bs);
	for (i=0; i<ptr->segment_run_table_count; i++) {
		GF_AdobeSegmentRunTableBox *asrt = NULL;
		e = gf_isom_box_parse((GF_Box **)&asrt, bs);
		if (e) {
			if (asrt) gf_isom_box_del((GF_Box*)asrt);
			gf_free(tmp_str);
			return e;
		}
		gf_list_add(ptr->segment_run_table_entries, asrt);
	}

	ISOM_DECREASE_SIZE(ptr, 1)
	ptr->fragment_run_table_count = gf_bs_read_u8(bs);
	for (i=0; i<ptr->fragment_run_table_count; i++) {
		GF_AdobeFragmentRunTableBox *afrt = NULL;
		e = gf_isom_box_parse((GF_Box **)&afrt, bs);
		if (e) {
			if (afrt) gf_isom_box_del((GF_Box*)afrt);
			gf_free(tmp_str);
			return e;
		}
		gf_list_add(ptr->fragment_run_table_entries, afrt);
	}

	gf_free(tmp_str);

	return GF_OK;
}