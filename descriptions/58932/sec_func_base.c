static int
cflex_delete_file(sc_profile_t *profile, sc_pkcs15_card_t *p15card, sc_file_t *df)
{
        sc_path_t  path;
        sc_file_t  *parent;
        int             r = 0;
        /* Select the parent DF */
        path = df->path;
		if (path.len < 2) {
			return SC_ERROR_INVALID_ARGUMENTS;
		}
        path.len -= 2;
        r = sc_select_file(p15card->card, &path, &parent);
        if (r < 0)
                return r;

        r = sc_pkcs15init_authenticate(profile, p15card, parent, SC_AC_OP_DELETE);
        sc_file_free(parent);
        if (r < 0)
                return r;

	/* cryptoflex has no ERASE AC */
        memset(&path, 0, sizeof(path));
        path.type = SC_PATH_TYPE_FILE_ID;
        path.value[0] = df->id >> 8;
        path.value[1] = df->id & 0xFF;
        path.len = 2;

	r = sc_delete_file(p15card->card, &path);
	return r;
}