if (r < 0)
		LOG_TEST_RET(ctx, r, "ASN.1 decoding failed");

	if (*ecpoint_data != 0x04)
		LOG_TEST_RET(ctx, SC_ERROR_NOT_SUPPORTED, "Supported only uncompressed EC pointQ value");

	key->ecpointQ.len = ecpoint_len;