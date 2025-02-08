case FILE_PSTRING:
		l = 0;
		v = file_strncmp(magicentry->value.s, p->s, CAST(size_t, magicentry->vallen),
		    magicentry->str_flags);
		break;

	case FILE_BESTRING16:
	case FILE_LESTRING16:
		l = 0;
		v = file_strncmp16(magicentry->value.s, p->s, CAST(size_t, magicentry->vallen),
		    magicentry->str_flags);
		break;

	case FILE_SEARCH: { /* search ms->search.s for the string m->value.s */
		size_t slen;
		size_t idx;

		if (ms->search.s == NULL)
			return 0;

		slen = MIN(magicentry->vallen, sizeof(magicentry->value.s));
		l = 0;
		v = 0;
#ifdef HAVE_MEMMEM
		if (slen > 0 && magicentry->str_flags == 0) {
			const char *found;
			idx = magicentry->str_range + slen;
			if (magicentry->str_range == 0 || ms->search.s_len < idx)
				idx = ms->search.s_len;
			found = CAST(const char *, memmem(ms->search.s, idx,
			    magicentry->value.s, slen));
			if (!found)
				return 0;
			idx = found - ms->search.s;
			ms->search.offset += idx;
			ms->search.rm_len = ms->search.s_len - idx;
			break;
		}
#endif

		for (idx = 0; magicentry->str_range == 0 || idx < magicentry->str_range; idx++) {
			if (slen + idx > ms->search.s_len)
				return 0;

			v = file_strncmp(magicentry->value.s, ms->search.s + idx, slen,
			    magicentry->str_flags);
			if (v == 0) {	/* found match */
				ms->search.offset += idx;
				ms->search.rm_len = ms->search.s_len - idx;
				break;
			}
		}
		break;
	}