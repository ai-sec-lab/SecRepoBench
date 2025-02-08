static int
sc_oberthur_parse_privateinfo (struct sc_pkcs15_card *p15card,
		unsigned char *buff, size_t len, int postpone_allowed)
{
	struct sc_context *ctx = p15card->card->ctx;
	size_t ii;
	int rv;
	// <MASK>

	LOG_FUNC_RETURN(ctx, SC_SUCCESS);
}