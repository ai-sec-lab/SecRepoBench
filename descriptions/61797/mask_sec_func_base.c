int
iasecc_se_parse(struct sc_card *card, unsigned char *data, size_t data_len, struct iasecc_se_info *se)
{
	struct sc_context *ctx = card->ctx;
	size_t size, offs, size_size;
	int rv;

	LOG_FUNC_CALLED(ctx);

	if (*data == IASECC_SDO_TEMPLATE_TAG)   {
		size_size = iasecc_parse_size(data + 1, &size);
		LOG_TEST_RET(ctx, size_size, "parse error: invalid size data of IASECC_SDO_TEMPLATE");

		data += size_size + 1;
		data_len = size;
		sc_log(ctx,
		       "IASECC_SDO_TEMPLATE: size %"SC_FORMAT_LEN_SIZE_T"u, size_size %"SC_FORMAT_LEN_SIZE_T"u",
		       size, size_size);

		if (*data != IASECC_SDO_TAG_HEADER)
			LOG_FUNC_RETURN(ctx, SC_ERROR_INVALID_DATA);

		if ((*(data + 1) & 0x7F) != IASECC_SDO_CLASS_SE)
			 LOG_FUNC_RETURN(ctx, SC_ERROR_INVALID_DATA);

		size_size = iasecc_parse_size(data + 3, &size);
		LOG_TEST_RET(ctx, size_size, "parse error: invalid SDO SE data size");

		if (data_len != size + size_size + 3)
			LOG_TEST_RET(ctx, SC_ERROR_INVALID_DATA, "parse error: invalid SDO SE data size");

		data += 3 + size_size;
		data_len = size;
		sc_log(ctx,
		       "IASECC_SDO_TEMPLATE SE: size %"SC_FORMAT_LEN_SIZE_T"u, size_size %"SC_FORMAT_LEN_SIZE_T"u",
		       size, size_size);
	}

	if (*data != IASECC_SDO_CLASS_SE)   {
		sc_log(ctx,
		       "Invalid SE tag 0x%X; data length %"SC_FORMAT_LEN_SIZE_T"u",
		       *data, data_len);
		LOG_FUNC_RETURN(ctx, SC_ERROR_UNKNOWN_DATA_RECEIVED);
	}

	size_size = iasecc_parse_size(data + 1, &size);
	LOG_TEST_RET(ctx, size_size, "parse error: invalid size data");

	// <MASK>

	if (offs != data_len)
		LOG_TEST_RET(ctx, SC_ERROR_INVALID_DATA, "parse error: not totally parsed");

	LOG_FUNC_RETURN(ctx, SC_SUCCESS);
}