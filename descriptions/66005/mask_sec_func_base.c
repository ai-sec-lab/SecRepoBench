GF_Err RTSP_UnpackURL(char *sURL, char Server[1024], u16 *Port, char Service[1024], Bool *useTCP, char User[1024], char Pass[1024])
{
	char schema[10], *test, text[1024], *retest, *sep, *service_start;
	u32 i, len;
	Bool is_ipv6;
	if (!sURL) return GF_BAD_PARAM;

	Server[0] = 0;
	Service[0] = 0;
	User[0] = 0;
	Pass[0] = 0;

	*Port = 0;
	*useTCP = GF_FALSE;

	if (!strchr(sURL, ':')) return GF_BAD_PARAM;

	sep = strchr(sURL, '?');
	if (sep) sep[0] = 0;
	// <MASK>
	if (sep) sep[0] = '?';
	return GF_BAD_PARAM;

found:
	schema[i] = 0;
	if (stricmp(schema, "rtsp") && stricmp(schema, "rtspu") && stricmp(schema, "rtsph")  && stricmp(schema, "rtsps") && stricmp(schema, "satip")) return GF_URL_ERROR;
	//check for user/pass - not allowed
	/*
		test = strstr(sURL, "@");
		if (test) return GF_NOT_SUPPORTED;
	*/
	test = strstr(sURL, "://");
	if (!test) {
		if (sep) sep[0] = '?';
		return GF_URL_ERROR;
	}
	test += 3;
	//check for service
	retest = strstr(test, "/");
	if (!retest) {
		if (sep) sep[0] = '?';
		return GF_URL_ERROR;
	}
	if (!stricmp(schema, "rtsp") || !stricmp(schema, "satip") || !stricmp(schema, "rtsph") || !stricmp(schema, "rtsps"))
		*useTCP = GF_TRUE;

	service_start = retest;
	//check for port
	char *port = strrchr(test, ':');
	retest = (port<retest) ? port : NULL;
	/*IPV6 address*/
	if (retest && strchr(retest, ']')) retest = NULL;

	if (retest && strstr(retest, "/")) {
		retest += 1;
		i=0;
		while (i<strlen(retest)) {
			if (retest[i] == '/') break;
			text[i] = retest[i];
			i += 1;
		}
		text[i] = 0;
		*Port = atoi(text);
	}

	char *sep_auth = strchr(test, '@');
	if (sep_auth>service_start) sep_auth=NULL;
	if (sep_auth) {
		sep_auth[0] = 0;
		char *psep = strchr(test, ':');
		if (psep) psep[0] = 0;
		strncpy(User, test, 1023);
		User[1023]=0;
		if (psep) {
			strncpy(Pass, psep+1, 1023);
			Pass[1023]=0;
			if (psep) psep[0] = ':';
		}

		sep_auth[0] = '@';
		test = sep_auth+1;
	}

	//get the server name
	is_ipv6 = GF_FALSE;
	len = (u32) strlen(test);
	i=0;
	while (i<len) {
		if (test[i]=='[') is_ipv6 = GF_TRUE;
		else if (test[i]==']') is_ipv6 = GF_FALSE;
		if ( (test[i] == '/') || (!is_ipv6 && (test[i] == ':')) ) break;
		text[i] = test[i];
		i += 1;
	}
	text[i] = 0;
	strncpy(Server, text, 1023);
	Server[1023]=0;
	if (sep) sep[0] = '?';

	if (service_start) {
		strncpy(Service, service_start+1, 1023);
		Service[1023]=0;
	} else {
		Service[0]=0;
	}
	return GF_OK;
}