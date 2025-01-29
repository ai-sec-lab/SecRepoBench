const char *end;
			size_t lines, linecnt, bytecnt;

			if (s == NULL || nbytes < offset) {
				ms->search.s_len = 0;
				ms->search.s = NULL;
				return 0;
			}