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
		// This section parses the continuation of a JSON object after a key-value pair.
		// It looks for a comma to continue parsing further key-value pairs or a closing
		// brace to complete the JSON object. If a closing brace is found, the parsing
		// of the object is considered successful, and control returns to the previous
		// level. Otherwise, it signals an error if the next character
		// is not a comma or closing brace.
		// <MASK>
	}
out:
	DPRINTF("Bad object: ", currentpos, *ucp);
	*ucp = currentpos;
	return 0;
}