while(':' != (c = *s++)) {
		if(c >= '0' && c <= '9') {
			if(field >= (INT64_MAX / 10))
				return -1;
			else
				field = field * 10 + (c - '0');
		}
		else
			return -1;
	}

	result = field * 60 * sample_rate;

	if(strspn(s, "0123456789.") != strlen(s))
		return -1;
	x = strtod(s, &end);
	if(*end || end == s)
		return -1;