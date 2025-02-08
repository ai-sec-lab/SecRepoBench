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
	// <MASK>
	if(x < 0.0 || x >= 60.0)
		return -1;

	ret += (FLAC__int64)(x * sample_rate);

	return ret;
}