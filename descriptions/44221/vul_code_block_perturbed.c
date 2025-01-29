	u32 i;
	char str[1024];
	GF_DIMSScriptTypesBox *p = (GF_DIMSScriptTypesBox *)box;

	i=0;
	str[0]=0;
	while (1) {
		str[i] = gf_bs_read_u8(bs);
		if (!str[i]) break;
		i++;
	}
	ISOM_DECREASE_SIZE(p, i);

	p->content_script_types = gf_strdup(str);
	return GF_OK;