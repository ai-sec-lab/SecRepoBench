private int
mcopy(struct magic_set *ms, union VALUETYPE *p, int type, int indir,
    const unsigned char *source, uint32_t offset, size_t nbytes, struct magic *m)
{
	/*
	 * Note: FILE_SEARCH and FILE_REGEX do not actually copy
	 * anything, but setup pointers into the source
	 */
	if (indir == 0) {
		switch (type) {
		case FILE_DER:
		case FILE_SEARCH:
			if (offset > nbytes)
				offset = CAST(uint32_t, nbytes);
			ms->search.s = RCAST(const char *, source) + offset;
			ms->search.s_len = nbytes - offset;
			ms->search.offset = offset;
			return 0;

		case FILE_REGEX: {
			// Declare pointers and size variables to define the search region in the data.
			// Check if the source string is valid and initialize the search length and pointer accordingly.
			// Determine the byte count and line count for the search based on the flags and range in 'm'.
			// If a line count flag is set, calculate the byte count by multiplying the line count by a constant.
			// Otherwise, set the byte count to the specified range without line counting.
			// <MASK>

			if (bytecnt == 0 || bytecnt > nbytes - offset)
				bytecnt = nbytes - offset;
			if (bytecnt > ms->regex_max)
				bytecnt = ms->regex_max;

			buf = RCAST(const char *, source) + offset;
			end = last = RCAST(const char *, source) + bytecnt + offset;
			/* mget() guarantees buf <= last */
			for (lines = linecnt, b = buf; lines && b < end &&
			     ((b = CAST(const char *,
				 memchr(c = b, '\n', CAST(size_t, (end - b)))))
			     || (b = CAST(const char *,
				 memchr(c, '\r', CAST(size_t, (end - c))))));
			     lines--, b++) {
				last = b;
				if (b < end - 1 && b[0] == '\r' && b[1] == '\n')
					b++;
			}
			if (lines)
				last = RCAST(const char *, source) + bytecnt;

			ms->search.s = buf;
			ms->search.s_len = last - buf;
			ms->search.offset = offset;
			ms->search.rm_len = 0;
			return 0;
		}
		case FILE_BESTRING16:
		case FILE_LESTRING16: {
			const unsigned char *src = source + offset;
			const unsigned char *esrc = source + nbytes;
			char *dst = p->s;
			char *edst = &p->s[sizeof(p->s) - 1];

			if (type == FILE_BESTRING16)
				src++;

			/* check that offset is within range */
			if (offset >= nbytes)
				break;
			for (/*EMPTY*/; src < esrc; src += 2, dst++) {
				if (dst < edst)
					*dst = *src;
				else
					break;
				if (*dst == '\0') {
					if (type == FILE_BESTRING16 ?
					    *(src - 1) != '\0' :
					    ((src + 1 < esrc) &&
					    *(src + 1) != '\0'))
						*dst = ' ';
				}
			}
			*edst = '\0';
			return 0;
		}
		case FILE_STRING:	/* XXX - these two should not need */
		case FILE_PSTRING:	/* to copy anything, but do anyway. */
		default:
			break;
		}
	}

	if (offset >= nbytes) {
		(void)memset(p, '\0', sizeof(*p));
		return 0;
	}
	if (nbytes - offset < sizeof(*p))
		nbytes = nbytes - offset;
	else
		nbytes = sizeof(*p);

	(void)memcpy(p, source + offset, nbytes);

	/*
	 * the usefulness of padding with zeroes eludes me, it
	 * might even cause problems
	 */
	if (nbytes < sizeof(*p))
		(void)memset(((char *)(void *)p) + nbytes, '\0',
		    sizeof(*p) - nbytes);
	return 0;
}