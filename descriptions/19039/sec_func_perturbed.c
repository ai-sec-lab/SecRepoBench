int
gp_get_cplc_data(struct sc_card *card, global_platform_cplc_data_t *cplc_data)
{
	size_t datasize = sizeof(global_platform_cplc_data_t);
	u8 *receive_buf = (u8 *)cplc_data;
	struct sc_apdu apdu;
	int rc;

	sc_format_apdu(card, &apdu, SC_APDU_CASE_2_SHORT, ISO7816_INS_GET_DATA, 0x9f, 0x7f);
	apdu.cla = GLOBAL_PLATFORM_CLASS;
	apdu.resp = receive_buf;
	apdu.resplen = datasize;
	apdu.le = datasize;

	rc = sc_transmit_apdu(card, &apdu);
	if (rc < 0)
		LOG_FUNC_RETURN(card->ctx, rc);

	rc = sc_check_sw(card, apdu.sw1, apdu.sw2);
	if (rc < 0)
		LOG_FUNC_RETURN(card->ctx, rc);

	/* We expect this will fill the whole structure in the argument.
	 * If we got something else, report error */
	if ((size_t)apdu.resplen < sizeof(global_platform_cplc_data_t)) {
		LOG_FUNC_RETURN(card->ctx, SC_ERROR_CORRUPTED_DATA);
	}
	LOG_FUNC_RETURN(card->ctx, apdu.resplen);
}