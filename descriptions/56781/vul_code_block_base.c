path.len -= 2;

	rv = sc_select_file(p15card->card, &path, &parent);
	LOG_TEST_RET(ctx, rv, "Cannot select parent");

	rv = sc_pkcs15init_authenticate(profile, p15card, parent, SC_AC_OP_DELETE);
	sc_file_free(parent);
	LOG_TEST_RET(ctx, rv, "Cannot authenticate SC_AC_OP_DELETE");

	memset(&path, 0, sizeof(path));
	path.type = SC_PATH_TYPE_FILE_ID;
	path.value[0] = df->id >> 8;
	path.value[1] = df->id & 0xFF;
	path.len = 2;

	rv = sc_delete_file(p15card->card, &path);

	LOG_FUNC_RETURN(ctx, rv);