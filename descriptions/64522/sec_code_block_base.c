size_t ivlen = 8;
	int r;

	sc_copy_asn1_entry(c_asn1_des_iv, asn1_des_iv);
	sc_format_asn1_entry(asn1_des_iv + 0, iv, &ivlen, 0);
	r = _sc_asn1_decode(ctx, asn1_des_iv, buf, buflen, NULL, NULL, 0, depth + 1);
	if (r < 0)
		return r;
	if (ivlen != 8)
		return SC_ERROR_INVALID_ASN1_OBJECT;
	*paramp = malloc(8);
	if (!*paramp)
		return SC_ERROR_OUT_OF_MEMORY;
	memcpy(*paramp, iv, 8);
	return 0;