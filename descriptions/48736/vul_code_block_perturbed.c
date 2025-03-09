for (; uc < ue && --length;) {
		if (*uc++ != *++str) {
			DPRINTF("Bad const: ", uc, *ucp);
			return 0;
		}
	}