static int entersafe_gen_key(sc_card_t *securitycard, sc_entersafe_gen_key_data *data)
{
	int	r;
	size_t len = data->key_length >> 3;
	sc_apdu_t apdu;
	u8 rbuf[300] = {0};
	u8 sbuf[4],*p;
	size_t plen = 0;

	SC_FUNC_CALLED(securitycard->ctx, SC_LOG_DEBUG_VERBOSE);

	/* MSE */
	sc_format_apdu(securitycard, &apdu, SC_APDU_CASE_3_SHORT, 0x22, 0x01, 0xB8);
	apdu.lc=0x04;
	sbuf[0]=0x83;
	sbuf[1]=0x02;
	sbuf[2]=data->key_id;
	sbuf[3]=0x2A;
	apdu.data = sbuf;
	apdu.datalen=4;
	apdu.lc=4;
	apdu.le=0;

	r=entersafe_transmit_apdu(securitycard, &apdu, 0,0,0,0);
	LOG_TEST_RET(securitycard->ctx, r, "APDU transmit failed");
	LOG_TEST_RET(securitycard->ctx, sc_check_sw(securitycard,apdu.sw1,apdu.sw2),"EnterSafe set MSE failed");

	/* generate key */
	sc_format_apdu(securitycard, &apdu, SC_APDU_CASE_3_SHORT, 0x46,  0x00, 0x00);
	apdu.le      = 0;
	sbuf[0] = (u8)(data->key_length >> 8);
	sbuf[1] = (u8)(data->key_length);
	apdu.data    = sbuf;
	apdu.lc      = 2;
	apdu.datalen = 2;

	r = entersafe_transmit_apdu(securitycard, &apdu,0,0,0,0);
	LOG_TEST_RET(securitycard->ctx, r, "APDU transmit failed");
	LOG_TEST_RET(securitycard->ctx, sc_check_sw(securitycard,apdu.sw1,apdu.sw2),"EnterSafe generate key pair failed");

	/* read public key via READ PUBLIC KEY */
	sc_format_apdu(securitycard, &apdu, SC_APDU_CASE_2_SHORT, 0xE6,  0x2A, data->key_id);
	apdu.cla     = 0x80;
	apdu.resp    = rbuf;
	apdu.resplen = sizeof(rbuf);
	apdu.le      = 256;
	r = entersafe_transmit_apdu(securitycard, &apdu,0,0,0,0);
	LOG_TEST_RET(securitycard->ctx, r, "APDU transmit failed");
	LOG_TEST_RET(securitycard->ctx, sc_check_sw(securitycard,apdu.sw1,apdu.sw2),"EnterSafe get pukey failed");

	p = rbuf;
	plen = apdu.resplen;
	if (*p != 'E') {
		SC_FUNC_RETURN(securitycard->ctx, SC_LOG_DEBUG_VERBOSE, SC_ERROR_INVALID_DATA);
	}
	if ((size_t)(p - rbuf) + 2 + p[1] >= plen) {
		SC_FUNC_RETURN(securitycard->ctx, SC_LOG_DEBUG_VERBOSE, SC_ERROR_INVALID_DATA);
	}
	p += 2 + p[1];
	/* N */
	if (*p != 'N') {
		SC_FUNC_RETURN(securitycard->ctx, SC_LOG_DEBUG_VERBOSE, SC_ERROR_INVALID_DATA);
	}
	if ((size_t)(p - rbuf) + 2 >= plen) {
		SC_FUNC_RETURN(securitycard->ctx, SC_LOG_DEBUG_VERBOSE, SC_ERROR_INVALID_DATA);
	}
	++p;
	if (*p++ > 0x80)
	{
		u8 len_bytes = (*(p - 1)) & 0x0f;
		size_t module_len = 0;
		if ((size_t)(p - rbuf) + len_bytes >= plen) {
			SC_FUNC_RETURN(securitycard->ctx, SC_LOG_DEBUG_VERBOSE, SC_ERROR_INVALID_DATA);
		}
		 while (len_bytes != 0)
		 {
			  module_len = module_len << 8;
			  module_len += *p++;
			  --len_bytes;
		 }
	}

	if ((p - rbuf) + len >= plen) {
		SC_FUNC_RETURN(securitycard->ctx, SC_LOG_DEBUG_VERBOSE, SC_ERROR_INVALID_DATA);
	}

	data->modulus = malloc(len);
	if (!data->modulus)
		SC_FUNC_RETURN(securitycard->ctx, SC_LOG_DEBUG_VERBOSE, SC_ERROR_OUT_OF_MEMORY);
	entersafe_reverse_buffer(p, len);
	memcpy(data->modulus, p, len);

	SC_FUNC_RETURN(securitycard->ctx, SC_LOG_DEBUG_VERBOSE, SC_SUCCESS);
}