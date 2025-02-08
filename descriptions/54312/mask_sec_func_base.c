int msc_partial_update_object(sc_card_t *card, msc_id objectId, int offset, const u8 *data, size_t dataLength)
{
	u8 buffer[MSC_MAX_APDU];
	sc_apdu_t apdu;
	int r;

	// <MASK>
}