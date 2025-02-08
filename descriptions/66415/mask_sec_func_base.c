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
	//extract the schema
	i = 0;
	while (i<=strlen(sURL)) {
		if (i==10) return GF_BAD_PARAM;
		if (sURL[i] == ':') goto found;
		schema[i] = sURL[i];
		i += 1;
	}
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

	// <MASK>
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