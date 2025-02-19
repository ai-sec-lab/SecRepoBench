static int
json_parse_array(const unsigned char **ucp, const unsigned char *ue,
	size_t *st, size_t lvl)
{
	const unsigned char *currentposition = *ucp;

	DPRINTF("Parse array: ", currentposition, *ucp);
	while (currentposition < ue) {
		currentposition = json_skip_space(currentposition, ue);
		// Check for the end of a JSON array during parsing. The code verifies
		// if the current character being parsed is the closing bracket ']' 
		// indicating the end of the array.
		// <MASK>
		if (!json_parse(&currentposition, ue, st, lvl + 1))
			goto out;
		if (currentposition == ue)
			goto out;
		switch (*currentposition) {
		case ',':
			currentposition++;
			continue;
		case ']':
		done:
			st[JSON_ARRAYN]++;
			DPRINTF("Good array: ", currentposition, *ucp);
			*ucp = currentposition + 1;
			return 1;
		default:
			goto out;
		}
	}
out:
	DPRINTF("Bad array: ", currentposition,  *ucp);
	*ucp = currentposition;
	return 0;
}