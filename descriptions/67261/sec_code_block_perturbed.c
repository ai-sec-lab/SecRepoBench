char *dst = NULL;
	char *res=NULL;
	u32 res_size=0;

	GF_Err e = gf_utf_get_string_from_bom((char *)data, buffer_length, &dst, &res, &res_size);
	if (e) return NULL;

	data = res;
	//strip all spaces and \r\n\t
	while (data[0] && strchr("\n\r\t ", (char) data[0])) {
		data++;
		res_size--;
	}

#define PROBE_OK(_score, _mime) \
		*score = _score;\
		if (dst) gf_free(dst);\
		return _mime; \