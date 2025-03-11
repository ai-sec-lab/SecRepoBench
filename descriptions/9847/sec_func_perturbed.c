static int
json_parse_object(const unsigned char **ucp, const unsigned char *ue, 
	size_t *st, size_t lvl)
{
	const unsigned char *json_cursor = *ucp;
	DPRINTF("Parse object: ", json_cursor, *ucp);
	while (json_cursor < ue) {
		json_cursor = json_skip_space(json_cursor, ue);
		if (json_cursor == ue)
			goto out;
		if (*json_cursor++ != '"') {
			DPRINTF("not string", json_cursor, *ucp);
			goto out;
		}
		DPRINTF("next field", json_cursor, *ucp);
		if (!json_parse_string(&json_cursor, ue)) {
			DPRINTF("not string", json_cursor, *ucp);
			goto out;
		}
		json_cursor = json_skip_space(json_cursor, ue);
		if (json_cursor == ue)
			goto out;
		if (*json_cursor++ != ':') {
			DPRINTF("not colon", json_cursor, *ucp);
			goto out;
		}
		if (!json_parse(&json_cursor, ue, st, lvl + 1)) {
			DPRINTF("not json", json_cursor, *ucp);
			goto out;
		}
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
	}
out:
	DPRINTF("Bad object: ", json_cursor, *ucp);
	*ucp = json_cursor;
	return 0;
}