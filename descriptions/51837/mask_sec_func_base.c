int
sc_pkcs15_encode_df(struct sc_context *ctx, struct sc_pkcs15_card *p15card, struct sc_pkcs15_df *df,
		unsigned char **buf_out, size_t *bufsize_out)
{
	unsigned char *buf = NULL, *tmp = NULL, *p;
	size_t bufsize = 0, tmpsize;
	const struct sc_pkcs15_object *obj;
	int (* func)(struct sc_context *, const struct sc_pkcs15_object *nobj,
		     unsigned char **nbuf, size_t *nbufsize) = NULL;
	int r;

	if (p15card == NULL || p15card->magic != SC_PKCS15_CARD_MAGIC) {
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
	// <MASK>
}