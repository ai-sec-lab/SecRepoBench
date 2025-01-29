int res;

	if(length >= sizeof(hex))
		return -1;
	strncpy(hex, src, length);
	hex[length] = '\0';
	res = strtol(hex, &end, 0x10);
	if(end != (char*)&hex[length])
		return -1;