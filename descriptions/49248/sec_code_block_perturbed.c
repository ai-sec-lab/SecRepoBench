const u8  *p, *q, *tag;
	int       r;
	size_t    fids = 0, len;

	SC_FUNC_CALLED(sccard->ctx, SC_LOG_DEBUG_VERBOSE);

	/* 0x16: DIRECTORY */
	/* 0x02: list both DF and EF */

get_next_part:
	sc_format_apdu(sccard, &apdu, SC_APDU_CASE_2_SHORT, 0x16, 0x02, offset);
	apdu.cla = 0x80;
	apdu.le = 256;
	apdu.resplen = 256;
	apdu.resp = rbuf;

	r = sc_transmit_apdu(sccard, &apdu);
	LOG_TEST_RET(sccard->ctx, r, "APDU transmit failed");
	r = sc_check_sw(sccard, apdu.sw1, apdu.sw2);
	LOG_TEST_RET(sccard->ctx, r, "DIRECTORY command returned error");

	if (apdu.resplen > 256) {
		sc_log(sccard->ctx,  "directory listing > 256 bytes, cutting");
	}

	p = rbuf;