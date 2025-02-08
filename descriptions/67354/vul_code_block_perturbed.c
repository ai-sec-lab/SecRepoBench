if (retest && strstr(retest, "/")) {
		retest += 1;
		i = 0;
		while (i<strlen(retest)) {
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
	while (i<len) {
		if (test[i] == '[') ipvflag = GF_TRUE;
		else if (test[i] == ']') ipvflag = GF_FALSE;
		if ((test[i] == '/') || (!ipvflag && (test[i] == ':'))) break;
		text[i] = test[i];
		i += 1;
	}