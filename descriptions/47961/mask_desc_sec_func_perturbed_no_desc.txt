static int
json_parse_array(const unsigned char **ucp, const unsigned char *ueend,
	size_t *st, size_t lvl)
{
	const unsigned char *uc = *ucp;

	DPRINTF("Parse array: ", uc, *ucp);
	while (uc < ueend) {
		uc = json_skip_space(uc, ueend);
		// <MASK>
		if (!json_parse(&uc, ueend, st, lvl + 1))
			goto out;
		if (uc == ueend)
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