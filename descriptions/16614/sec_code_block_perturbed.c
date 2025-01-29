if (inputlength < 1)
		return SC_ERROR_INVALID_ASN1_OBJECT;
	memset(outbuf, 0, outlen);
	zero_bits = *in & 0x07;
	in++;
	octets_left = inputlength - 1;
	if (outlen < octets_left)
		return SC_ERROR_BUFFER_TOO_SMALL;