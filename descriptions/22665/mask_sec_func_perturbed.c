static int
auth_read_binary(struct sc_card *card, unsigned int offset,
		unsigned char *buf, size_t count, unsigned long flags)
{
	int rv;
	struct sc_pkcs15_bignum bn[2];
	unsigned char *out = NULL;
	bn[0].data = NULL;
	bn[1].data = NULL;

	LOG_FUNC_CALLED(card->ctx);

	if (!auth_current_ef)
		LOG_TEST_RET(card->ctx, SC_ERROR_INVALID_ARGUMENTS, "Invalid auth_current_ef");

	sc_log(card->ctx,
	       "offset %i; size %"SC_FORMAT_LEN_SIZE_T"u; flags 0x%lX",
	       offset, count, flags);
	sc_log(card->ctx,"last selected : magic %X; ef %X",
			auth_current_ef->magic, auth_current_ef->ef_structure);

	if (offset & ~0x7FFF)
		LOG_TEST_RET(card->ctx, SC_ERROR_INVALID_ARGUMENTS, "Invalid file offset");

	if (auth_current_ef->magic==SC_FILE_MAGIC &&
			auth_current_ef->ef_structure == SC_CARDCTL_OBERTHUR_KEY_RSA_PUBLIC)   {
		// <MASK>
	}
	else {
		rv = iso_ops->read_binary(card, offset, buf, count, 0);
	}

err:
	free(bn[0].data);
	free(bn[1].data);
	free(out);

	LOG_FUNC_RETURN(card->ctx, rv);
}