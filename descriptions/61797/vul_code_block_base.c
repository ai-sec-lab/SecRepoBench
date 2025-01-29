if (data_len != size + size_size + 1)
		LOG_TEST_RET(ctx, SC_ERROR_INVALID_DATA, "parse error: invalid SE data size");

	offs = 1 + size_size;
	for (; offs < data_len;)   {
		rv = iasecc_crt_parse(card, data + offs, se);
		LOG_TEST_RET(ctx, rv, "parse error: invalid SE data");

		offs += rv;
	}