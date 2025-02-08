static int
json_parse_string(const unsigned char **ucp, const unsigned char *ue)
{
	const unsigned char *uc = *ucp;
	size_t i;

	DPRINTF("Parse string: ", uc, *ucp);
	while (uc < ue) {
		switch (*uc++) {
		case '\0':
			goto out;
		case '\\':
			// <MASK>
		case '"':
			*ucp = uc;
			return 1;
		default:
			continue;
		}
	}
out:
	DPRINTF("Bad string: ", uc, *ucp);
	*ucp = uc;
	return 0;
}