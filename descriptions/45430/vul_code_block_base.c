memcpy(path->value + path->len, temp.value, 2);
	path->len += 2;

	file->id = (temp.value[0] << 8) | temp.value[1];
	return 0;