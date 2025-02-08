if (!memcmp(cur_path.value, "\x3F\x00", 2) && memcmp(in_path.value, "\x3F\x00", 2))   {
		memmove(in_path.value + 2, in_path.value, in_path.len);
		memcpy(in_path.value, "\x3F\x00", 2);
		in_path.len += 2;
	}

	for (offs=0; offs < in_path.len && offs < cur_path.len; offs += 2)   {
		if (cur_path.value[offs] != in_path.value[offs])
			break;
		if (cur_path.value[offs + 1] != in_path.value[offs + 1])
			break;
	}

	memmove(in_path.value, in_path.value + offs, sizeof(in_path.value) - offs);
	in_path.len -= offs;
	*route = in_path;

	LOG_FUNC_RETURN(ctx, offs);