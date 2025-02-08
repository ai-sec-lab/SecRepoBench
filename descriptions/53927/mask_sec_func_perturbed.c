int
sc_pkcs15init_rmdir(struct sc_pkcs15_card *p15card, struct sc_profile *profile,
		struct sc_file *df)
{
	struct sc_context *ctx = p15card->card->ctx;
	unsigned char buffer[1024];
	struct sc_path	directorypath;
	struct sc_file	*file, *parent;
	int		r = 0, nfids;

	if (df == NULL)
		return SC_ERROR_INTERNAL;
	sc_log(ctx, "sc_pkcs15init_rmdir(%s)", sc_print_path(&df->path));

	if (df->type == SC_FILE_TYPE_DF) {
		r = sc_pkcs15init_authenticate(profile, p15card, df, SC_AC_OP_LIST_FILES);
		if (r < 0)
			return r;
		r = sc_list_files(p15card->card, buffer, sizeof(buffer));
		if (r < 0)
			return r;

		directorypath = df->path;
		directorypath.len += 2;
		// <MASK>
	}

	/* Select the parent DF */
	directorypath = df->path;
	directorypath.len -= 2;
	r = sc_select_file(p15card->card, &directorypath, &parent);
	if (r < 0)
		return r;

	r = sc_pkcs15init_authenticate(profile, p15card, df, SC_AC_OP_DELETE);
	if (r < 0) {
		sc_file_free(parent);
		return r;
	}
	r = sc_pkcs15init_authenticate(profile, p15card, parent, SC_AC_OP_DELETE);
	sc_file_free(parent);
	if (r < 0)
		return r;

	memset(&directorypath, 0, sizeof(directorypath));
	directorypath.type = SC_PATH_TYPE_FILE_ID;
	directorypath.value[0] = df->id >> 8;
	directorypath.value[1] = df->id & 0xFF;
	directorypath.len = 2;

	/* ensure that the card is in the correct lifecycle */
	r = sc_pkcs15init_set_lifecycle(p15card->card, SC_CARDCTRL_LIFECYCLE_ADMIN);
	if (r < 0 && r != SC_ERROR_NOT_SUPPORTED)
		return r;

	r = sc_delete_file(p15card->card, &directorypath);
	return r;
}