if (json_cursor == ue)
			goto out;
		switch (*json_cursor++) {
		case ',':
			continue;
		case '}': /* { */
			*ucp = json_cursor;
			DPRINTF("Good object: ", json_cursor, *ucp);
			return 1;
		default:
			*ucp = json_cursor - 1;
			DPRINTF("not more", json_cursor, *ucp);
			goto out;
		}