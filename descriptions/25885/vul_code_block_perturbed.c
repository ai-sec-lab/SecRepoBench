int res;

	if(length >= sizeof(hex))
		return -1;
	strncpy(hex, src, length+1);
	hex[length] = '\0';
	res = strtol(hex, &end, 0x10);
	if(end != (char*)&hex[length])
		return -1;