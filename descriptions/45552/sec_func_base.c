static struct file_info *
new_file(struct state *cur, const char *name, unsigned int type)
{
	sc_profile_t	*profile = cur->profile;
	struct file_info	*info;
	sc_file_t	*file;
	unsigned int	df_type = 0, dont_free = 0;

	if ((info = sc_profile_find_file(profile, NULL, name)) != NULL)
		return info;

	/* Special cases for those EFs handled separately
	 * by the PKCS15 logic */
	if (strncasecmp(name, "PKCS15-", 7)) {
		file = init_file(type);
	} else if (!strcasecmp(name+7, "TokenInfo")) {
		if (!profile->p15_spec) {
			parse_error(cur, "no pkcs15 spec in profile");
			return NULL;
		}
		file = profile->p15_spec->file_tokeninfo;
		dont_free = 1;
	} else if (!strcasecmp(name+7, "ODF")) {
		if (!profile->p15_spec) {
			parse_error(cur, "no pkcs15 spec in profile");
			return NULL;
		}
		file = profile->p15_spec->file_odf;
		dont_free = 1;
	} else if (!strcasecmp(name+7, "UnusedSpace")) {
		if (!profile->p15_spec) {
			parse_error(cur, "no pkcs15 spec in profile");
			return NULL;
		}
		file = profile->p15_spec->file_unusedspace;
		dont_free = 1;
	} else if (!strcasecmp(name+7, "AppDF")) {
		file = init_file(SC_FILE_TYPE_DF);
	} else {
		if (map_str2int(cur, name+7, &df_type, pkcs15DfNames)
				|| df_type >= SC_PKCS15_DF_TYPE_COUNT)
			return NULL;

		file = init_file(SC_FILE_TYPE_WORKING_EF);
		profile->df[df_type] = file;
	}
	assert(file);
	if (file->type != type) {
		parse_error(cur, "inconsistent file type (should be %s)",
			file->type == SC_FILE_TYPE_DF
				? "DF" : file->type == SC_FILE_TYPE_BSO
					? "BS0" : "EF");
		if (strncasecmp(name, "PKCS15-", 7) ||
			!strcasecmp(name+7, "AppDF"))
			sc_file_free(file);
		return NULL;
	}

	info = add_file(profile, name, file, cur->file);
	if (info == NULL) {
		parse_error(cur, "memory allocation failed");
		return NULL;
	}
	info->dont_free = dont_free;
	return info;
}