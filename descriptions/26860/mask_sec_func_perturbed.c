static int
authentic_reduce_path(struct sc_card *card, struct sc_path *route)
{
	struct sc_context *ctx = card->ctx;
	struct sc_path in_path, cur_path;
	size_t offs;

	LOG_FUNC_CALLED(ctx);

	if (!route || route->len <= 2 || route->type == SC_PATH_TYPE_DF_NAME)
		LOG_FUNC_RETURN(ctx, SC_SUCCESS);

	if (!card->cache.valid || !card->cache.current_df)
		LOG_FUNC_RETURN(ctx, 0);

	in_path = *route;
	cur_path = card->cache.current_df->path;

	// <MASK>
}