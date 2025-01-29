if (offs + 2 + len > info_len) {
		free(info_blob);
		LOG_TEST_RET(ctx, SC_ERROR_INVALID_DATA, "Failed to add public key: invalid 'Label' length");
	} else if (len) {
		if (len > sizeof(key_obj.label) - 1)
			len = sizeof(key_obj.label) - 1;
		memcpy(key_obj.label, info_blob + offs + 2, len);
	}
	offs += 2 + len;

	/* ID */
	if (offs + 2 > info_len) {
		free(info_blob);
		LOG_TEST_RET(ctx, SC_ERROR_UNKNOWN_DATA_RECEIVED, "Failed to add public key: no 'ID'");
	}
	len = *(info_blob + offs + 1) + *(info_blob + offs) * 0x100;
	if (len == 0
			|| len > sizeof(key_info.id.value)
			|| offs + 2 + len > info_len) {
		free(info_blob);
		LOG_TEST_RET(ctx, SC_ERROR_INVALID_DATA, "Failed to add public key: invalid 'ID' length");
	}
	memcpy(key_info.id.value, info_blob + offs + 2, len);
	key_info.id.len = len;

	free(info_blob);

	/* Ignore Start/End dates */

	snprintf(ch_tmp, sizeof(ch_tmp), "%s%04X", AWP_OBJECTS_DF_PUB, file_id);
	sc_format_path(ch_tmp, &key_info.path);

	key_info.native = 1;
	key_info.key_reference = file_id & 0xFF;
	key_info.modulus_length = size;

	rv = sc_pkcs15emu_add_rsa_pubkey(p15card, &key_obj, &key_info);

	LOG_FUNC_RETURN(ctx, rv);