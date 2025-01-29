	if (outlen < octets_left)
		return SC_ERROR_BUFFER_TOO_SMALL;
	if (inlen < 1)
		return SC_ERROR_INVALID_ASN1_OBJECT;

	zero_bits = *in & 0x07;
	octets_left = inlen - 1;
	in++;
	memset(outbuf, 0, outlen);