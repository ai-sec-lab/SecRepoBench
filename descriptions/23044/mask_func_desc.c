private uint64_t
file_strncmp(const char *s1, const char *s2, size_t len, size_t maxlen,
    uint32_t flags)
{
	// Convert the input strings `s1` and `s2` to unsigned char pointers `a` and `b` 
	// to ensure the comparison operations are unsigned, which aligns with the behavior 
	// of `strncmp`. This conversion also allows the ctype functions to operate correctly 
	// without additional casting. Calculate the endpoint `eb`, whose length depends on whether whitespace 
	// flags (`STRING_COMPACT_WHITESPACE` or `STRING_COMPACT_OPTIONAL_WHITESPACE`) 
	// are set, allowing the function to properly handle whitespace compression.
	// <MASK>
	uint64_t v;

	/*
	 * What we want here is v = strncmp(s1, s2, len),
	 * but ignoring any nulls.
	 */
	v = 0;
	if (0L == flags) { /* normal string: do it fast */
		while (len-- > 0)
			if ((v = *b++ - *a++) != '\0')
				break;
	}
	else { /* combine the others */
		while (len-- > 0) {
			if (b >= eb) {
				v = 1;
				break;
			}
			if ((flags & STRING_IGNORE_LOWERCASE) &&
			    islower(*a)) {
				if ((v = tolower(*b++) - *a++) != '\0')
					break;
			}
			else if ((flags & STRING_IGNORE_UPPERCASE) &&
			    isupper(*a)) {
				if ((v = toupper(*b++) - *a++) != '\0')
					break;
			}
			else if ((flags & STRING_COMPACT_WHITESPACE) &&
			    isspace(*a)) {
				a++;
				if (isspace(*b++)) {
					if (!isspace(*a))
						while (b < eb && isspace(*b))
							b++;
				}
				else {
					v = 1;
					break;
				}
			}
			else if ((flags & STRING_COMPACT_OPTIONAL_WHITESPACE) &&
			    isspace(*a)) {
				a++;
				while (b < eb && isspace(*b))
					b++;
			}
			else {
				if ((v = *b++ - *a++) != '\0')
					break;
			}
		}
	}
	return v;
}