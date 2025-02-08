int
sc_pkcs15_decode_pubkey_ec(sc_context_t *ctx,
		struct sc_pkcs15_pubkey_ec *key,
		const u8 *buffer, size_t buflen)
{
	int r;
	u8 * ecpoint_data;
	size_t ecpoint_len;
	struct sc_asn1_entry asn1_ec_pointQ[C_ASN1_EC_POINTQ_SIZE];

	LOG_FUNC_CALLED(ctx);
	sc_copy_asn1_entry(c_asn1_ec_pointQ, asn1_ec_pointQ);
	sc_format_asn1_entry(asn1_ec_pointQ + 0, &ecpoint_data, &ecpoint_len, 1);
	r = sc_asn1_decode(ctx, asn1_ec_pointQ, buffer, buflen, NULL, NULL);
	if (r < 0)
		LOG_TEST_RET(ctx, r, "ASN.1 decoding failed");

	if (ecpoint_len == 0 || *ecpoint_data != 0x04)
		LOG_TEST_RET(ctx, SC_ERROR_NOT_SUPPORTED, "Supported only uncompressed EC pointQ value");

	key->ecpointQ.len = ecpoint_len;
	key->ecpointQ.value = ecpoint_data;

	/*
	 * Only get here if raw point is stored in pkcs15 without curve name
	 * spki has the curvename, so we can get the field_length
	 * Following only true for curves that are multiple of 8
	 */
	key->params.field_length = (ecpoint_len - 1)/2 * 8;
	LOG_FUNC_RETURN(ctx, SC_SUCCESS);
}