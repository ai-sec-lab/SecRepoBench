memcpy(path->value + path->len, temppath.value, 2);
	path->len += 2;

	file->id = (temppath.value[0] << 8) | temppath.value[1];
	return 0;