if (uc == ue)
			goto out;
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