static int tcos_decipher(sc_card_t *card, const u8 * ciphergram, size_t crgram_len, u8 * out, size_t outlen)
{
	sc_context_t *ctx;
	sc_apdu_t apdu;
	u8 rbuf[SC_MAX_APDU_BUFFER_SIZE];
	u8 sbuf[SC_MAX_APDU_BUFFER_SIZE];
	tcos_data *data;
	int tcos3, r;

	assert(card != NULL && ciphergram != NULL && out != NULL);
	ctx = card->ctx;
	tcos3=(card->type==SC_CARD_TYPE_TCOS_V3);
	data=(tcos_data *)card->drv_data;

	LOG_FUNC_CALLED(ctx);
	sc_log(ctx,
		"TCOS3:%d PKCS1:%d\n",tcos3,
		!!(data->pad_flags & SC_ALGORITHM_RSA_PAD_PKCS1));

	sc_format_apdu(card, &apdu, crgram_len>255 ? SC_APDU_CASE_4_EXT : SC_APDU_CASE_4_SHORT, 0x2A, 0x80, 0x86);
	apdu.resp = rbuf;
	apdu.resplen = sizeof(rbuf);
	apdu.le = crgram_len;

	apdu.data = sbuf;
	apdu.lc = apdu.datalen = crgram_len+1;
	sbuf[0] = tcos3 ? 0x00 : ((data->pad_flags & SC_ALGORITHM_RSA_PAD_PKCS1) ? 0x81 : 0x02);
	// <MASK>
}