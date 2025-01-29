for (; uc < ue && --len;) {
		if (*uc++ != *++str) {
			DPRINTF("Bad const: ", uc, *ucp);
			return 0;
		}
	}
	DPRINTF("Good const: ", uc, *ucp);
	return 1;