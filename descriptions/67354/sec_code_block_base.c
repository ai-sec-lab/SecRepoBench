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
	is_ipv6 = GF_FALSE;
	len = (u32)strlen(test);
	i = 0;
	while (i<len && i<GF_ARRAY_LENGTH(text)) {
		if (test[i] == '[') is_ipv6 = GF_TRUE;
		else if (test[i] == ']') is_ipv6 = GF_FALSE;
		if ((test[i] == '/') || (!is_ipv6 && (test[i] == ':'))) break;
		text[i] = test[i];
		i += 1;
	}