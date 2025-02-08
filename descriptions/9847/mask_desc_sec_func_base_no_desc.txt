static int
json_parse_object(const unsigned char **ucp, const unsigned char *ue, 
	size_t *st, size_t lvl)
{
	const unsigned char *uc = *ucp;
	DPRINTF("Parse object: ", uc, *ucp);
	while (uc < ue) {
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
		// <MASK>
	}
out:
	DPRINTF("Bad object: ", uc, *ucp);
	*ucp = uc;
	return 0;
}