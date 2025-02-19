static int
json_parse_array(const unsigned char **ucp, const unsigned char *ue,
	size_t *st, size_t lvl)
{
	const unsigned char *uc = *ucp;

	DPRINTF("Parse array: ", uc, *ucp);
	while (uc < ue) {
		uc = json_skip_space(uc, ue);
		// Check for the end of a JSON array during parsing. The code verifies
		// if the current character being parsed is the closing bracket ']' 
		// indicating the end of the array.
		// <MASK>
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
	}
out:
	DPRINTF("Bad array: ", uc,  *ucp);
	*ucp = uc;
	return 0;
}