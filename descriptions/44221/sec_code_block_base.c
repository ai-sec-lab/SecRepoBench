GF_DIMSScriptTypesBox *p = (GF_DIMSScriptTypesBox *)s;

	p->content_script_types = gf_malloc(sizeof(char) * (s->size+1));
	if (!p->content_script_types) return GF_OUT_OF_MEM;
	gf_bs_read_data(bs, p->content_script_types, s->size);
	p->content_script_types[s->size] = 0;
	return GF_OK;