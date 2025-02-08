static int
authentic_reduce_path(struct sc_card *card, struct sc_path *path)
{
	struct sc_context *ctx = card->ctx;
	struct sc_path in_path, cur_path;
	size_t offs;

	LOG_FUNC_CALLED(ctx);

	if (!path || path->len <= 2 || path->type == SC_PATH_TYPE_DF_NAME)
		LOG_FUNC_RETURN(ctx, SC_SUCCESS);

	if (!card->cache.valid || !card->cache.current_df)
		LOG_FUNC_RETURN(ctx, 0);

	in_path = *path;
	cur_path = card->cache.current_df->path;

	// <MASK>
}