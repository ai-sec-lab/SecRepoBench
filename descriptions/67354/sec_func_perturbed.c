void rtpin_satip_get_server_ip(const char *sURL, char *Server)
{
	char schema[10], *test, text[1024], *retest;
	u32 i, len;
	Bool ipvflag;
	if (!sURL) return;

	strcpy(Server, "");

	//extract the schema
	i = 0;
	while (i < strlen(sURL)) {
		if (sURL[i] == ':')
			goto found;
		schema[i] = sURL[i];
		i += 1;
	}
	return;

found:
	schema[MIN(i, GF_ARRAY_LENGTH(schema)-1)] = 0;
	if (stricmp(schema, "satip")) {
		GF_LOG(GF_LOG_ERROR, GF_LOG_RTP, ("[RTP] Wrong SATIP schema %s - not setting up\n", schema));
		return;
	}
	test = strstr(sURL, "://");
	test += 3;

	//check for port
	retest = strrchr(test, ':');
	/*IPV6 address*/
	if (retest && strchr(retest, ']')) retest = NULL;

	if (retest && strstr(retest, "/")) {
		retest += 1;
		i = 0;
		while (i<strlen(retest) && i<GF_ARRAY_LENGTH(text)) {
			if (retest[i] == '/') break;
			text[i] = retest[i];
			i += 1;
		}
		text[MIN(i, GF_ARRAY_LENGTH(text)-1)] = 0;
	}
	//get the server name
	ipvflag = GF_FALSE;
	len = (u32)strlen(test);
	i = 0;
	while (i<len && i<GF_ARRAY_LENGTH(text)) {
		if (test[i] == '[') ipvflag = GF_TRUE;
		else if (test[i] == ']') ipvflag = GF_FALSE;
		if ((test[i] == '/') || (!ipvflag && (test[i] == ':'))) break;
		text[i] = test[i];
		i += 1;
	}
	text[MIN(i, GF_ARRAY_LENGTH(text)-1)] = 0;
	strcpy(Server, text);
}