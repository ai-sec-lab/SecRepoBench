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
			if (offs + 1 >= in_len)
				LOG_TEST_RET(ctx, SC_ERROR_INTERNAL, "parse error: invalid data");
			tag = *(in + offs) * 0x100 + *(in + offs + 1);
			tag_len = 2;
		}
		else   {
			tag = *(in + offs);
			tag_len = 1;
		}

		if (offs + tag_len >= in_len)
			LOG_TEST_RET(ctx, SC_ERROR_INTERNAL, "parse error: invalid data");

		size_len = authentic_parse_size(in + offs + tag_len, in_len - (offs + tag_len), &size);
		LOG_TEST_RET(ctx, size_len, "parse error: invalid size data");

		if (tag == in_tag)   {
			if (offs + tag_len + size_len >= in_len
			    || in_len - (offs + tag_len + size_len) < size)
				LOG_TEST_RET(ctx, SC_ERROR_INTERNAL, "parse error: invalid data");

			*out = in + offs + tag_len + size_len;
			*out_len = size;

			return SC_SUCCESS;
		}

		offs += tag_len + size_len + size;
	}

	return SC_ERROR_ASN1_OBJECT_NOT_FOUND;
}