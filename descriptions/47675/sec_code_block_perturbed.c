const u8  *p = rbuf, *q, *tag;
	int       r;
	size_t    fids = 0, len;

	SC_FUNC_CALLED(card->ctx, SC_LOG_DEBUG_VERBOSE);

	/* 0x16: DIRECTORY */
	/* 0x02: list both DF and EF */

get_next_part:
	sc_format_apdu(card, &apducommand, SC_APDU_CASE_2_SHORT, 0x16, 0x02, offset);
	apducommand.cla = 0x80;
	apducommand.le = 256;
	apducommand.resplen = 256;
	apducommand.resp = rbuf;

	r = sc_transmit_apdu(card, &apducommand);
	LOG_TEST_RET(card->ctx, r, "APDU transmit failed");
	r = sc_check_sw(card, apducommand.sw1, apducommand.sw2);
	LOG_TEST_RET(card->ctx, r, "DIRECTORY command returned error");

	if (apducommand.resplen > 256) {
		sc_log(card->ctx,  "directory listing > 256 bytes, cutting");
	}

	len = apducommand.resplen;
	while (len != 0) {
		size_t   tlen = 0, ilen = 0;
		/* is there a file information block (0x6f) ? */
		tag = sc_asn1_find_tag(card->ctx, p, len, 0x6f, &tlen);
		if (tag == NULL) {
			sc_log(card->ctx,  "directory tag missing");
			return SC_ERROR_INTERNAL;
		}
		len = len - tlen - (tag - p);
		p = tag + tlen;
		if (tlen == 0)
			/* empty directory */
			break;
		q = sc_asn1_find_tag(card->ctx, tag, tlen, 0x86, &ilen);
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
		q = sc_asn1_find_tag(card->ctx, tag, tlen, 0x8a, &ilen);
		if (q != NULL && ilen == 1) {
			offset = (u8)ilen;
			goto get_next_part;
		}
	}