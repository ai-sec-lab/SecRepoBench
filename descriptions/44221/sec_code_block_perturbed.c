GF_DIMSScriptTypesBox *p = (GF_DIMSScriptTypesBox *)box;

	p->content_script_types = gf_malloc(sizeof(char) * (box->size+1));
	if (!p->content_script_types) return GF_OUT_OF_MEM;
	gf_bs_read_data(bs, p->content_script_types, box->size);
	p->content_script_types[box->size] = 0;
	return GF_OK;