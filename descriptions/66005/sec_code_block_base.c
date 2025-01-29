//extract the schema
	i = 0;
	while (i<=strlen(sURL)) {
		if (i==10) return GF_BAD_PARAM;
		if (sURL[i] == ':') goto found;
		schema[i] = sURL[i];
		i += 1;
	}