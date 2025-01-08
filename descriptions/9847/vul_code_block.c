
		uc = json_skip_space(uc, ue);
		if (uc == ue)
			goto out;
		if (*uc++ != '"') {
			DPRINTF("not string", uc, *ucp);
			goto out;
		}
		DPRINTF("next field", uc, *ucp);
		if (!json_parse_string(&uc, ue)) {
			DPRINTF("not string", uc, *ucp);
			goto out;
		}
		uc = json_skip_space(uc, ue);
		if (uc == ue)
			goto out;
		if (*uc++ != ':') {
			DPRINTF("not colon", uc, *ucp);
			goto out;
		}
		if (!json_parse(&uc, ue, st, lvl + 1)) {
			DPRINTF("not json", uc, *ucp);
			goto out;
		}
		switch (*uc++) {
		case ',':
			continue;
		case '}': /* { */
			*ucp = uc;
			DPRINTF("Good object: ", uc, *ucp);
			return 1;
		default:
			*ucp = uc - 1;
			DPRINTF("not more", uc, *ucp);
			goto out;
		}
	