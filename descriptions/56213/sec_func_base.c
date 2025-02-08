static int sc_hsm_write_ef(sc_card_t *card,
			       int fid,
			       unsigned int idx, const u8 *buf, size_t count)
{
	sc_context_t *ctx = card->ctx;
	sc_apdu_t apdu;
	u8 *cmdbuff, *p;
	size_t len;
	int r;

	if (idx > 0xffff) {
		sc_log(ctx,  "invalid EF offset: 0x%X > 0xFFFF", idx);
		return SC_ERROR_OFFSET_TOO_LARGE;
	}

	cmdbuff = malloc(8 + count);
	if (!cmdbuff) {
		LOG_FUNC_RETURN(card->ctx, SC_ERROR_OUT_OF_MEMORY);
	}

	size_t bytes_left = count;
	// 8 bytes are required for T54(4) and T53(4)
	size_t blk_size = card->max_send_size - 8;
	size_t to_send = 0;
	size_t file_offset = (size_t) idx;
	size_t offset = 0;
	do {
		len = 0;
		to_send = bytes_left >= blk_size ? blk_size : bytes_left;
		p = cmdbuff;
		// ASN1 0x54 offset
		*p++ = 0x54;
		*p++ = 0x02;
		*p++ = (file_offset >> 8) & 0xFF;
		*p++ = file_offset & 0xFF;
		// ASN1 0x53 to_send
		*p++ = 0x53;
		if (to_send < 128) {
			*p++ = (u8)to_send;
			len = 6;
		} else if (to_send < 256) {
			*p++ = 0x81;
			*p++ = (u8)to_send;
			len = 7;
		} else {
			*p++ = 0x82;
			*p++ = (to_send >> 8) & 0xFF;
			*p++ = to_send & 0xFF;
			len = 8;
		}

		if (buf != NULL)
			memcpy(p, buf+offset, to_send);
		len += to_send;

		sc_format_apdu(card, &apdu, SC_APDU_CASE_3, 0xD7, fid >> 8, fid & 0xFF);
		apdu.data = cmdbuff;
		apdu.datalen = len;
		apdu.lc = len;

		r = sc_transmit_apdu(card, &apdu);
		LOG_TEST_GOTO_ERR(ctx, r, "APDU transmit failed");
		r = sc_check_sw(card, apdu.sw1, apdu.sw2);
		LOG_TEST_GOTO_ERR(ctx, r, "Check SW error");

		bytes_left -= to_send;
		offset += to_send;
		file_offset += to_send;
	} while (0 < bytes_left);

err:
	free(cmdbuff);

	LOG_FUNC_RETURN(ctx, count);
}