static int
authentic_get_tagged_data(struct sc_context *ctx, unsigned char *in, size_t in_len,
		unsigned in_tag, unsigned char **out, size_t *out_len)
{
	size_t size_len, tag_len, offs, size;
	unsigned tag;

	if (!out || !out_len)
		LOG_FUNC_RETURN(ctx, SC_ERROR_INVALID_ARGUMENTS);

	for (offs = 0; offs < in_len; )   {
		if ((*(in + offs) == 0x7F) || (*(in + offs) == 0x5F))   {
			tag = *(in + offs) * 0x100 + *(in + offs + 1);
			tag_len = 2;
		}
		else   {
			tag = *(in + offs);
			tag_len = 1;
		}

		// <MASK>

		offs += tag_len + size_len + size;
	}

	return SC_ERROR_ASN1_OBJECT_NOT_FOUND;
}