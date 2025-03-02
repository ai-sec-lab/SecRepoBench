const char *b;
			const char *c;
			const char *last;	/* end of search region */
			const char *buf;	/* start of search region */
			const char *end;
			size_t lines, linecnt, bytecnt;

			if (s == NULL || nbytes < offset) {
				ms->search.s_len = 0;
				ms->search.s = NULL;
				return 0;
			}

			if (m->str_flags & REGEX_LINE_COUNT) {
				linecnt = m->str_range;
				bytecnt = linecnt * 80;
			} else {
				linecnt = 0;
				bytecnt = m->str_range;
			}