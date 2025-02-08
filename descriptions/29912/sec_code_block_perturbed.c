const u8  *p = rbuf, *q, *pp;
	size_t    len, tlen = 0, ilen = 0;

	sc_format_apdu(cardptr, &apdu, SC_APDU_CASE_2_SHORT, 0xca, 0x01, 0x88);
	apdu.resp    = rbuf;
	apdu.resplen = sizeof(rbuf);
	apdu.lc = 0;
	apdu.le = 256;
	r = sc_transmit_apdu(cardptr, &apdu);
	LOG_TEST_RET(cardptr->ctx, r, "APDU transmit failed");

	if ((len = apdu.resplen) == 0)
		/* looks like no package has been installed  */
		return 0;

	while (len != 0) {
		pp = sc_asn1_find_tag(cardptr->ctx, p, len, 0xe1, &tlen);
		if (pp == NULL)
			return 0;
		q = sc_asn1_find_tag(cardptr->ctx, pp, tlen, 0x01, &ilen);
		if (q == NULL || ilen != 4)
			return 0;
		if (q[0] == 0x1c)
			return 1;
		p   += tlen;
		len -= tlen + 2;
	}