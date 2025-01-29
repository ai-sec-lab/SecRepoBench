if (*uc == ']')
			goto done;
		if (!json_parse(&uc, ue, st, lvl + 1))
			goto out;
		if (uc == ue)
			goto out;
		switch (*uc) {
		case ',':
			uc++;
			continue;
		case ']':
		done:
			st[JSON_ARRAYN]++;
			DPRINTF("Good array: ", uc, *ucp);
			*ucp = uc + 1;
			return 1;
		default:
			goto out;
		}