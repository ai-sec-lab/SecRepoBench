int msc_partial_update_object(sc_card_t *smartCard, msc_id objectId, int offset, const u8 *data, size_t dataLength)
{
	u8 buffer[MSC_MAX_APDU];
	sc_apdu_t apdu;
	int r;

	if (dataLength + 9 > MSC_MAX_APDU)
		return SC_ERROR_INVALID_ARGUMENTS;

	sc_format_apdu(smartCard, &apdu, SC_APDU_CASE_3_SHORT, 0x54, 0x00, 0x00);
	apdu.lc = dataLength + 9;
	if (smartCard->ctx->debug >= 2)
		sc_log(smartCard->ctx, 
			 "WRITE: Offset: %x\tLength: %"SC_FORMAT_LEN_SIZE_T"u\n",
			 offset, dataLength);
	
	memcpy(buffer, objectId.id, 4);
	ulong2bebytes(buffer + 4, offset);
	buffer[8] = (u8)dataLength;
	memcpy(buffer + 9, data, dataLength);
	apdu.data = buffer;
	apdu.datalen = apdu.lc;
	r = sc_transmit_apdu(smartCard, &apdu);
	LOG_TEST_RET(smartCard->ctx, r, "APDU transmit failed");
	if(apdu.sw1 == 0x90 && apdu.sw2 == 0x00)
		return dataLength;
	if(apdu.sw1 == 0x9C) {
		if(apdu.sw2 == 0x07) {
			SC_FUNC_RETURN(smartCard->ctx, SC_LOG_DEBUG_VERBOSE, SC_ERROR_FILE_NOT_FOUND);
		} else if(apdu.sw2 == 0x06) {
			SC_FUNC_RETURN(smartCard->ctx, SC_LOG_DEBUG_VERBOSE, SC_ERROR_NOT_ALLOWED);
		} else if(apdu.sw2 == 0x0F) {
			/* GUESSED */
			SC_FUNC_RETURN(smartCard->ctx, SC_LOG_DEBUG_VERBOSE, SC_ERROR_INVALID_ARGUMENTS);
		}
	}
	if (smartCard->ctx->debug >= 2) {
		sc_log(smartCard->ctx,  "got strange SWs: 0x%02X 0x%02X\n",
		     apdu.sw1, apdu.sw2);
	}
	return dataLength;
}