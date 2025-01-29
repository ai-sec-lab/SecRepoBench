if (retest && strstr(retest, "/")) {
		retest += 1;
		i=0;
		while (i<strlen(retest) && i<1023) {
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
	strncpy(Server, text, 1024);