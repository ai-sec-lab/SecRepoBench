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
	size_t blocksize = card->max_send_size - 8;
	size_t to_send = 0;
	// <MASK>

err:
	free(cmdbuff);

	LOG_FUNC_RETURN(ctx, count);
}