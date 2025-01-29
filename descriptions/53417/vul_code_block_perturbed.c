path.len -= 2;
	r = sc_select_file(p15card->card, &path, &parent);
	if (r < 0)
		return r;

	r = sc_pkcs15init_authenticate(userprofile, p15card, df, SC_AC_OP_DELETE);
	if (r < 0) {
		sc_file_free(parent);
		return r;
	}
	r = sc_pkcs15init_authenticate(userprofile, p15card, parent, SC_AC_OP_DELETE);
	sc_file_free(parent);
	if (r < 0)
		return r;

	memset(&path, 0, sizeof(path));
	path.type = SC_PATH_TYPE_FILE_ID;
	path.value[0] = df->id >> 8;
	path.value[1] = df->id & 0xFF;
	path.len = 2;

	/* ensure that the card is in the correct lifecycle */
	r = sc_pkcs15init_set_lifecycle(p15card->card, SC_CARDCTRL_LIFECYCLE_ADMIN);
	if (r < 0 && r != SC_ERROR_NOT_SUPPORTED)
		return r;

	r = sc_delete_file(p15card->card, &path);
	return r;