if (m->str_range != 0
			    && ms->search.s_len >= m->str_range + slen) {
				found = memmem(ms->search.s,
				    m->str_range + slen, m->value.s, slen);
			} else {
				found = memmem(ms->search.s,
				    ms->search.s_len, m->value.s, slen);
				if (!found)
					return 0;
			}
			if (!found) {
				v = 1;
			} else {
				idx = found - ms->search.s;
				ms->search.offset += idx;
				ms->search.rm_len = ms->search.s_len - idx;
			}
			break;