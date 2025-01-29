if (path->len + 2 > sizeof(path->value)) {
		parse_error(cur, "File path too long\n");
		return 1;
	}
	memcpy(path->value + path->len, temp.value, 2);
	path->len += 2;

	file->id = (temp.value[0] << 8) | temp.value[1];
	return 0;