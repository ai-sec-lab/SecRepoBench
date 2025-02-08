int
sc_pkcs15init_rmdir(struct sc_pkcs15_card *p15card, struct sc_profile *profile,
		struct sc_file *df)
{
	struct sc_context *ctx = p15card->card->ctx;
	unsigned char buffer[1024];
	struct sc_path	path;
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

		path = df->path;
		path.len += 2;
		if (path.len > SC_MAX_PATH_SIZE)
			return SC_ERROR_INTERNAL;

		nfids = r / 2;
		while (r >= 0 && nfids--) {
			path.value[path.len-2] = buffer[2*nfids];
			path.value[path.len-1] = buffer[2*nfids+1];
			r = sc_select_file(p15card->card, &path, &file);
			if (r < 0) {
				if (r == SC_ERROR_FILE_NOT_FOUND)
					continue;
				break;
			}
			r = sc_pkcs15init_rmdir(p15card, profile, file);
			sc_file_free(file);
		}

		if (r < 0)
			return r;
	}

	/* Select the parent DF */
	path = df->path;
	// <MASK>
}