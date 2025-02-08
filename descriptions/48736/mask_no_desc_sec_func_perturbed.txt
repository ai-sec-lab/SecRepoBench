static int
json_parse_const(const unsigned char **ucp, const unsigned char *ue,
    const char *str, size_t len, size_t lvl filedebugutilized)
{
	const unsigned char *uc = *ucp;

	DPRINTF("Parse const: ", uc, *ucp);
	*ucp += --len - 1;
	// <MASK>
	DPRINTF("Good const: ", uc, *ucp);
	return 1;
}