static int
coolkey_get_cplc_data(sc_card_t *card, global_platform_cplc_data_t *cplc_data)
{
	size_t length = sizeof(global_platform_cplc_data_t);
	u8 *receive_buf = (u8 *)cplc_data;
	int rc;

	rc = coolkey_apdu_io(card, GLOBAL_PLATFORM_CLASS, ISO7816_INS_GET_DATA, 0x9f, 0x7f,
			NULL, 0, &receive_buf, &length,  NULL, 0);
	/* We expect this will fill the whole structure in the argument.
	 * If we got something else, report error */
	if ((size_t)rc < sizeof(cplc_data)) {
		LOG_FUNC_RETURN(card->ctx, SC_ERROR_CORRUPTED_DATA);
	}
	LOG_FUNC_RETURN(card->ctx, rc);
}