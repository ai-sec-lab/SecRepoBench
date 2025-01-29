//extract the schema
	i = 0;
	while (i<=strlen(sURL)) {
		if (sURL[i] == ':') goto found;
		schema[i] = sURL[i];
		i += 1;
	}