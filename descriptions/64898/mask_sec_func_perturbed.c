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
	// <MASK>
}