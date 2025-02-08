int
cosm_delete_file(struct sc_pkcs15_card *p15card, struct sc_profile *profile,
		struct sc_file *df)
{
	struct sc_context *ctx = p15card->card->ctx;
	struct sc_path  path;
	struct sc_file  *parent;
	int rv = 0;

	SC_FUNC_CALLED(ctx, SC_LOG_DEBUG_VERBOSE);
	sc_log(ctx,  "id %04X", df->id);
	if (df->type==SC_FILE_TYPE_DF)   {
		rv = sc_pkcs15init_authenticate(profile, p15card, df, SC_AC_OP_DELETE);
		LOG_TEST_RET(ctx, rv, "Cannot authenticate SC_AC_OP_DELETE");
	}

	/* Select the parent DF */
	path = df->path;
	// <MASK>
}