static int tcos_compute_signature(sc_card_t *card, const u8 * data, size_t datalen, u8 * out, size_t outlen)
{
	size_t i, dlen=datalen;
	sc_apdu_t apdu;
	u8 rbuf[SC_MAX_APDU_BUFFER_SIZE];
	u8 sbuf[SC_MAX_APDU_BUFFER_SIZE];
	int tcos3, r;

	assert(card != NULL && data != NULL && out != NULL);
	tcos3=(card->type==SC_CARD_TYPE_TCOS_V3);

	// We can sign (key length / 8) bytes
	if (datalen > 256) SC_FUNC_RETURN(card->ctx, SC_LOG_DEBUG_VERBOSE, SC_ERROR_INVALID_ARGUMENTS);

	if(((tcos_data *)card->drv_data)->next_sign) // <MASK>
	apdu.resp = rbuf;
	apdu.resplen = sizeof(rbuf);
	apdu.le = tcos3 ? 256 : 128;
	apdu.data = sbuf;
	apdu.lc = apdu.datalen = dlen;

	r = sc_transmit_apdu(card, &apdu);
	LOG_TEST_RET(card->ctx, r, "APDU transmit failed");
	if (tcos3 && apdu.p1==0x80 && apdu.sw1==0x6A && apdu.sw2==0x87) {
		int keylen=128;
		sc_format_apdu(card, &apdu, SC_APDU_CASE_4_SHORT, 0x2A,0x80,0x86);
		for(i=0; i<sizeof(sbuf);++i) sbuf[i]=0xff;
		sbuf[0]=0x02; sbuf[1]=0x00; sbuf[2]=0x01; sbuf[keylen-datalen]=0x00;
		memcpy(sbuf+keylen-datalen+1, data, datalen);
		dlen=keylen+1;

		apdu.resp = rbuf;
		apdu.resplen = sizeof(rbuf);
		apdu.le = 128;
		apdu.data = sbuf;
		apdu.lc = apdu.datalen = dlen;
		r = sc_transmit_apdu(card, &apdu);
		LOG_TEST_RET(card->ctx, r, "APDU transmit failed");
	}
	if (apdu.sw1==0x90 && apdu.sw2==0x00) {
		size_t len = apdu.resplen>outlen ? outlen : apdu.resplen;
		memcpy(out, apdu.resp, len);
		SC_FUNC_RETURN(card->ctx, SC_LOG_DEBUG_VERBOSE, len);
	}
	SC_FUNC_RETURN(card->ctx, SC_LOG_DEBUG_VERBOSE, sc_check_sw(card, apdu.sw1, apdu.sw2));
}