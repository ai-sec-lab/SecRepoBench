char str[1024];
	u32 i;
	GF_DIMSSceneConfigBox *p = (GF_DIMSSceneConfigBox *)s;

	ISOM_DECREASE_SIZE(p, 3);
	p->profile = gf_bs_read_u8(bs);
	p->level = gf_bs_read_u8(bs);
	p->pathComponents = gf_bs_read_int(bs, 4);
	p->fullRequestHost = gf_bs_read_int(bs, 1);
	p->streamType = gf_bs_read_int(bs, 1);
	p->containsRedundant = gf_bs_read_int(bs, 2);

	i=0;
	str[0]=0;
	while (i < GF_ARRAY_LENGTH(str)) {
		str[i] = gf_bs_read_u8(bs);
		if (!str[i]) break;
		i++;
	}
	ISOM_DECREASE_SIZE(p, i);

	p->textEncoding = gf_strdup(str);

	i=0;
	str[0]=0;
	while (i < GF_ARRAY_LENGTH(str)) {
		str[i] = gf_bs_read_u8(bs);
		if (!str[i]) break;
		i++;
	}
	ISOM_DECREASE_SIZE(p, i);

	p->contentEncoding = gf_strdup(str);