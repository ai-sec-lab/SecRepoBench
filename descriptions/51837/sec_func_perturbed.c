int
sc_pkcs15_encode_df(struct sc_context *ctx, struct sc_pkcs15_card *pkcscard, struct sc_pkcs15_df *df,
		unsigned char **buf_out, size_t *bufsize_out)
{
	unsigned char *buf = NULL, *tmp = NULL, *p;
	size_t bufsize = 0, tmpsize;
	const struct sc_pkcs15_object *obj;
	int (* func)(struct sc_context *, const struct sc_pkcs15_object *nobj,
		     unsigned char **nbuf, size_t *nbufsize) = NULL;
	int r;

	if (pkcscard == NULL || pkcscard->magic != SC_PKCS15_CARD_MAGIC) {
		return SC_ERROR_INVALID_ARGUMENTS;
	}
	switch (df->type) {
	case SC_PKCS15_PRKDF:
		func = sc_pkcs15_encode_prkdf_entry;
		break;
	case SC_PKCS15_PUKDF:
	case SC_PKCS15_PUKDF_TRUSTED:
		func = sc_pkcs15_encode_pukdf_entry;
		break;
	case SC_PKCS15_SKDF:
		func = sc_pkcs15_encode_skdf_entry;
		break;
	case SC_PKCS15_CDF:
	case SC_PKCS15_CDF_TRUSTED:
	case SC_PKCS15_CDF_USEFUL:
		func = sc_pkcs15_encode_cdf_entry;
		break;
	case SC_PKCS15_DODF:
		func = sc_pkcs15_encode_dodf_entry;
		break;
	case SC_PKCS15_AODF:
		func = sc_pkcs15_encode_aodf_entry;
		break;
	}
	if (func == NULL) {
		sc_log(ctx, "unknown DF type: %d", df->type);
		*buf_out = NULL;
		*bufsize_out = 0;
		return 0;
	}
	for (obj = pkcscard->obj_list; obj != NULL; obj = obj->next) {
		if (obj->df != df)
			continue;
		r = func(ctx, obj, &tmp, &tmpsize);
		if (r) {
			free(tmp);
			free(buf);
			return r;
		}
		if (!tmpsize)
			continue;
		p = (u8 *) realloc(buf, bufsize + tmpsize);
		if (!p) {
			free(tmp);
			free(buf);
			return SC_ERROR_OUT_OF_MEMORY;
		}
		buf = p;
		memcpy(buf + bufsize, tmp, tmpsize);
		free(tmp);
		tmp = NULL;
		bufsize += tmpsize;
	}
	*buf_out = buf;
	*bufsize_out = bufsize;

	return 0;
}