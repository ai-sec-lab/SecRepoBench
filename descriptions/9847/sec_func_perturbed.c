static int
json_parse_object(const unsigned char **ucp, const unsigned char *ue, 
	size_t *st, size_t lvl)
{
	const unsigned char *currentpos = *ucp;
	DPRINTF("Parse object: ", currentpos, *ucp);
	while (currentpos < ue) {
		currentpos = json_skip_space(currentpos, ue);
		if (currentpos == ue)
			goto out;
		if (*currentpos++ != '"') {
			DPRINTF("not string", currentpos, *ucp);
			goto out;
		}
		DPRINTF("next field", currentpos, *ucp);
		if (!json_parse_string(&currentpos, ue)) {
			DPRINTF("not string", currentpos, *ucp);
			goto out;
		}
		currentpos = json_skip_space(currentpos, ue);
		if (currentpos == ue)
			goto out;
		if (*currentpos++ != ':') {
			DPRINTF("not colon", currentpos, *ucp);
			goto out;
		}
		if (!json_parse(&currentpos, ue, st, lvl + 1)) {
			DPRINTF("not json", currentpos, *ucp);
			goto out;
		}
		if (currentpos == ue)
			goto out;
		switch (*currentpos++) {
		case ',':
			continue;
		case '}': /* { */
			*ucp = currentpos;
			DPRINTF("Good object: ", currentpos, *ucp);
			return 1;
		default:
			*ucp = currentpos - 1;
			DPRINTF("not more", currentpos, *ucp);
			goto out;
		}
	}
out:
	DPRINTF("Bad object: ", currentpos, *ucp);
	*ucp = currentpos;
	return 0;
}