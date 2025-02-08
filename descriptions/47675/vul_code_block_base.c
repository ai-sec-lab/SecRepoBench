const u8  *p = rbuf, *q;
	int       r;
	size_t    fids = 0, len;

	SC_FUNC_CALLED(card->ctx, SC_LOG_DEBUG_VERBOSE);

	/* 0x16: DIRECTORY */
	/* 0x02: list both DF and EF */

get_next_part:
	sc_format_apdu(card, &apdu, SC_APDU_CASE_2_SHORT, 0x16, 0x02, offset);
	apdu.cla = 0x80;
	apdu.le = 256;
	apdu.resplen = 256;
	apdu.resp = rbuf;

	r = sc_transmit_apdu(card, &apdu);
	LOG_TEST_RET(card->ctx, r, "APDU transmit failed");
	r = sc_check_sw(card, apdu.sw1, apdu.sw2);
	LOG_TEST_RET(card->ctx, r, "DIRECTORY command returned error");

	if (apdu.resplen > 256) {
		sc_log(card->ctx,  "directory listing > 256 bytes, cutting");
	}

	len = apdu.resplen;
	while (len != 0) {
		size_t   tlen = 0, ilen = 0;
		/* is there a file information block (0x6f) ? */
		p = sc_asn1_find_tag(card->ctx, p, len, 0x6f, &tlen);
		if (p == NULL) {
			sc_log(card->ctx,  "directory tag missing");
			return SC_ERROR_INTERNAL;
		}
		if (tlen == 0)
			/* empty directory */
			break;
		q = sc_asn1_find_tag(card->ctx, p, tlen, 0x86, &ilen);
		if (q == NULL || ilen != 2) {
			sc_log(card->ctx,  "error parsing file id TLV object");
			return SC_ERROR_INTERNAL;
		}
		/* put file id in buf */
		if (buflen >= 2) {
			buf[fids++] = q[0];
			buf[fids++] = q[1];
			buflen -= 2;
		} else
			/* not enough space left in buffer => break */
			break;
		/* extract next offset */
		q = sc_asn1_find_tag(card->ctx, p, tlen, 0x8a, &ilen);
		if (q != NULL && ilen == 1) {
			offset = (u8)ilen;
			goto get_next_part;
		}
		len -= tlen + 2;
		p   += tlen;
	}