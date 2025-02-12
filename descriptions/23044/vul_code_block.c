/*
	 * Convert the source args to unsigned here so that (1) the
	 * compare will be unsigned as it is in strncmp() and (2) so
	 * the ctype functions will work correctly without extra
	 * casting.
	 */
	const unsigned char *a = RCAST(const unsigned char *, s1);
	const unsigned char *b = RCAST(const unsigned char *, s2);
	uint32_t ws = flags & (STRING_COMPACT_WHITESPACE |
	    STRING_COMPACT_OPTIONAL_WHITESPACE);
	const unsigned char *eb = b + (ws ? strlen(s2) : len);