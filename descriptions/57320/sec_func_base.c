static FLAC__int64 local__parse_ms_(const char *s, uint32_t sample_rate)
{
	FLAC__int64 ret, field;
	double x;
	char c, *end;

	c = *s++;
	if(c >= '0' && c <= '9')
		field = (c - '0');
	else
		return -1;
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

	ret = field * 60 * sample_rate;

	if(strspn(s, "0123456789.") != strlen(s))
		return -1;
	x = strtod(s, &end);
	if(*end || end == s)
		return -1;
	if(x < 0.0 || x >= 60.0)
		return -1;

	ret += (FLAC__int64)(x * sample_rate);

	return ret;
}