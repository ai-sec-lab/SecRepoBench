u8 zeroBuffer[MSC_MAX_APDU];
	size_t i;
	size_t max_write_unit = MIN(MSC_MAX_APDU, MSC_MAX_SEND - 9); /* - 9 for object ID+length */

	memset(zeroBuffer, 0, max_write_unit);
	for(i = 0; i < dataLength; i += max_write_unit) {
		int r = msc_partial_update_object(card, objectId, i, zeroBuffer, MIN(dataLength - i, max_write_unit));
		LOG_TEST_RET(card->ctx, r, "Error in zeroing file update");
	}