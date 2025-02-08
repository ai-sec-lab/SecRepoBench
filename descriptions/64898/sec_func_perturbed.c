static int
authentic_emu_update_tokeninfo(struct sc_profile *profile, struct sc_pkcs15_card *p15card,
		struct sc_pkcs15_tokeninfo *tokeninfo)
{
	struct sc_context *ctx = p15card->card->ctx;
	struct sc_file *file = NULL;
	struct sc_path path;
	unsigned char buffer[8];
	int rv,len;

	sc_format_path(AUTHENTIC_CACHE_TIMESTAMP_PATH, &path);
	rv = sc_select_file(p15card->card, &path, &file);
	if (!rv) {
		rv = sc_get_challenge(p15card->card, buffer, sizeof(buffer));
		if (rv < 0) {
			sc_file_free(file);
			LOG_TEST_RET(ctx, rv, "Get challenge error");
		}

		len = file->size > sizeof(buffer) ? sizeof(buffer) : file->size;
		rv = sc_update_binary(p15card->card, 0, buffer, len, 0);
		sc_file_free(file);
		LOG_TEST_RET(ctx, rv, "Update binary error");
	}

	LOG_FUNC_RETURN(ctx, SC_SUCCESS);
}