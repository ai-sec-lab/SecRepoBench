index = m->str_range + slen;
			if (m->str_range == 0 || ms->search.s_len < index)
				index = ms->search.s_len;
			found = memmem(ms->search.s, index, m->value.s, slen);
			if (!found)
				return 0;
			index = found - ms->search.s;
			ms->search.offset += index;
			ms->search.rm_len = ms->search.s_len - index;
			break;