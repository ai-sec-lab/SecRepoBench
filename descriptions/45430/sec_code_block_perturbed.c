if (path->len + 2 > sizeof(path->value)) {
		parse_error(cur, "File path too long\n");
		return 1;
	}
	memcpy(path->value + path->len, temppath.value, 2);
	path->len += 2;

	file->id = (temppath.value[0] << 8) | temppath.value[1];
	return 0;