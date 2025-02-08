static int sc_hsm_list_files(sc_card_t *card, u8 * buf, size_t buflen)
{
	sc_apdu_t apdu;
	u8 recvbuf[MAX_EXT_APDU_LENGTH];
	sc_hsm_private_data_t *priv = (sc_hsm_private_data_t *) card->drv_data;
	int r;

	if (priv->noExtLength) {
		sc_format_apdu(card, &apdu, SC_APDU_CASE_2, 0x58, 0, 0);
	} else {
		sc_format_apdu(card, &apdu, SC_APDU_CASE_2_EXT, 0x58, 0, 0);
	}
	apdu.cla = 0x80;
	apdu.resp = recvbuf;
	apdu.resplen = sizeof(recvbuf);
	apdu.le = 0;
	r = sc_transmit_apdu(card, &apdu);

	if ((r == SC_ERROR_TRANSMIT_FAILED) && (!priv->noExtLength)) {
		sc_log(card->ctx, "No extended length support ? Trying fall-back to short APDUs, probably breaking support for RSA 2048 operations");
		priv->noExtLength = 1;
		card->max_send_size = 248;		// 255 - 7 because of TLV in odd ins UPDATE BINARY
		return sc_hsm_list_files(card, buf, buflen);
	}
	LOG_TEST_RET(card->ctx, r, "ENUMERATE OBJECTS APDU transmit failed");

	// <MASK>
}