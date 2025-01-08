
	const unsigned char *uc = *ucp;

	DPRINTF("Parse const: ", uc, *ucp);
	*ucp += --len - 1;
	for (; uc < ue && --len;) {
		if (*uc++ != *++str) {
			DPRINTF("Bad const: ", uc, *ucp);
			return 0;
		}
	}
	DPRINTF("Good const: ", uc, *ucp);
	return 1;
