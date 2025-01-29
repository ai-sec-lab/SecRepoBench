if (offs > info_len) {
		free(info_blob);
		LOG_TEST_RET(ctx, SC_ERROR_UNKNOWN_DATA_RECEIVED, "Failed to add public key: no 'ID'");
	}
	len = *(info_blob + offs + 1) + *(info_blob + offs) * 0x100;
	if (!len || len > sizeof(key_info.id.value)) {
		free(info_blob);
		LOG_TEST_RET(ctx, SC_ERROR_INVALID_DATA, "Failed to add public key: invalid 'ID' length");
	}