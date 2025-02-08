if ((*(in + offs) == 0x7F) || (*(in + offs) == 0x5F))   {
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
			if (offs + tag_len + size_len >= in_len)
				LOG_TEST_RET(ctx, SC_ERROR_INTERNAL, "parse error: invalid data");

			*out = in + offs + tag_len + size_len;
			*out_len = size;

			return SC_SUCCESS;
		}