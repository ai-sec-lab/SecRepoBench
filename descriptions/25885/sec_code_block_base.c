int res;

	if(len >= sizeof(hex))
		return -1;
	strncpy(hex, src, len);
	hex[len] = '\0';
	res = strtol(hex, &end, 0x10);
	if(end != (char*)&hex[len])
		return -1;