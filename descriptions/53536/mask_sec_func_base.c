static int
authentic_get_tagged_data(struct sc_context *ctx, unsigned char *in, size_t in_len,
		unsigned in_tag, unsigned char **out, size_t *out_len)
{
	size_t size_len, tag_len, offs, size;
	unsigned tag;

	if (!out || !out_len)
		LOG_FUNC_RETURN(ctx, SC_ERROR_INVALID_ARGUMENTS);

	for (offs = 0; offs < in_len; )   {
		// <MASK>

		offs += tag_len + size_len + size;
	}

	return SC_ERROR_ASN1_OBJECT_NOT_FOUND;
}