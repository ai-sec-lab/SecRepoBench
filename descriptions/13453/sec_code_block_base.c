idx = m->str_range + slen;
			if (m->str_range == 0 || ms->search.s_len < idx)
				idx = ms->search.s_len;
			found = memmem(ms->search.s, idx, m->value.s, slen);
			if (!found)
				return 0;
			idx = found - ms->search.s;
			ms->search.offset += idx;
			ms->search.rm_len = ms->search.s_len - idx;
			break;