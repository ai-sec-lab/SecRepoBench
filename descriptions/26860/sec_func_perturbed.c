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

	if (!memcmp(cur_path.value, "\x3F\x00", 2) && memcmp(in_path.value, "\x3F\x00", 2))   {
		memmove(in_path.value + 2, in_path.value, (in_path.len - 2));
		memcpy(in_path.value, "\x3F\x00", 2);
		in_path.len += 2;
	}

	for (offs = 0; (offs + 1) < in_path.len && (offs + 1) < cur_path.len; offs += 2)   {
		if (cur_path.value[offs] != in_path.value[offs])
			break;
		if (cur_path.value[offs + 1] != in_path.value[offs + 1])
			break;
	}

	memmove(in_path.value, in_path.value + offs, sizeof(in_path.value) - offs);
	in_path.len -= offs;
	*route = in_path;

	LOG_FUNC_RETURN(ctx, offs);
}