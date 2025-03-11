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
		// This section parses the continuation of a JSON object after a key-value pair.
		// It looks for a comma to continue parsing further key-value pairs or a closing
		// brace to complete the JSON object. It signals an error if the next character
		// is not a comma or closing brace.
		// <MASK>
	}
out:
	DPRINTF("Bad object: ", json_cursor, *ucp);
	*ucp = json_cursor;
	return 0;
}