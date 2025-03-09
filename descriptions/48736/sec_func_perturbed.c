static int
json_parse_const(const unsigned char **ucp, const unsigned char *ue,
    const char *str, size_t length, size_t lvl __file_debugused)
{
	const unsigned char *uc = *ucp;

	DPRINTF("Parse const: ", uc, *ucp);
	*ucp += --length - 1;
	if (*ucp > ue)
		*ucp = ue;
	for (; uc < ue && --length;) {
		if (*uc++ != *++str) {
			DPRINTF("Bad const: ", uc, *ucp);
			return 0;
		}
	}
	DPRINTF("Good const: ", uc, *ucp);
	return 1;
}