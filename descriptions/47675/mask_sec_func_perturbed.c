static int cardos_list_files(sc_card_t *card, u8 *buf, size_t buflen)
{
	sc_apdu_t apducommand;
	u8        rbuf[256], offset = 0;
	// <MASK>

	r = fids;

	LOG_FUNC_RETURN(card->ctx, r);
}