u32 i, msize;
	GF_DIMSSceneConfigBox *p = (GF_DIMSSceneConfigBox *)s;

	ISOM_DECREASE_SIZE(p, 3);
	p->profile = gf_bs_read_u8(bitStream);
	p->level = gf_bs_read_u8(bitStream);
	p->pathComponents = gf_bs_read_int(bitStream, 4);
	p->fullRequestHost = gf_bs_read_int(bitStream, 1);
	p->streamType = gf_bs_read_int(bitStream, 1);
	p->containsRedundant = gf_bs_read_int(bitStream, 2);

	char *str = gf_malloc(sizeof(char)*(p->size+1));
	if (!str) return GF_OUT_OF_MEM;
	msize = p->size;
	str[msize] = 0;
	i=0;
	str[0]=0;
	while (i < msize) {
		str[i] = gf_bs_read_u8(bitStream);
		if (!str[i]) break;
		i++;
	}
	ISOM_DECREASE_SIZE(p, i);
	if (i == msize) {
		gf_free(str);
		return GF_ISOM_INVALID_FILE;
	}

	p->textEncoding = gf_strdup(str);

	i=0;
	str[0]=0;
	while (i < msize) {
		str[i] = gf_bs_read_u8(bitStream);
		if (!str[i]) break;
		i++;
	}
	ISOM_DECREASE_SIZE(p, i);
	if (i == msize) {
		gf_free(str);
		return GF_ISOM_INVALID_FILE;
	}

	p->contentEncoding = gf_strdup(str);
	gf_free(str);
	if (p->textEncoding || !p->contentEncoding)
		return GF_OUT_OF_MEM;