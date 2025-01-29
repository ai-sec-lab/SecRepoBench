rv = sc_select_file(p15card->card, &path, &file);
	if (!rv) {
		rv = sc_get_challenge(p15card->card, buffer, sizeof(buffer));
		if (!rv) {
			sc_file_free(file);
			LOG_TEST_RET(ctx, rv, "Get challenge error");
		}

		len = file->size > sizeof(buffer) ? sizeof(buffer) : file->size;
		rv = sc_update_binary(p15card->card, 0, buffer, len, 0);
		sc_file_free(file);
		LOG_TEST_RET(ctx, rv, "Update binary error");
	}

	LOG_FUNC_RETURN(ctx, SC_SUCCESS);