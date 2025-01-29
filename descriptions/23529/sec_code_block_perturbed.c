while (datalength > 0) {
		unsigned int amode, tlen = 3;
		if (datalength < 5 || p[0] != 0x80 || p[1] != 0x01) {
			sc_log(card->ctx,  "invalid access mode encoding");
			return SC_ERROR_INTERNAL;
		}
		amode = p[2];
		if (p[3] == 0x90 && p[4] == 0x00) {
			int r = set_sec_attr(file, amode, 0, SC_AC_NONE);
			if (r != SC_SUCCESS) 
				return r;
			tlen += 2;
		} else if (p[3] == 0x97 && p[4] == 0x00) {
			int r = set_sec_attr(file, amode, 0, SC_AC_NEVER);
			if (r != SC_SUCCESS) 
				return r;
			tlen += 2;
		} else if (p[3] == 0xA0 && p[4] > 0 && datalength >= 4U + p[4]) {
			/* TODO: support OR expressions */
			int r = set_sec_attr(file, amode, p[5], SC_AC_CHV);
			if (r != SC_SUCCESS)
				return r;
			tlen += 2 + p[4]; /* FIXME */
		} else if (p[3] == 0xAF && p[4] > 0 && datalength >= 4U + p[4]) {
			/* TODO: support AND expressions */
			int r = set_sec_attr(file, amode, p[5], SC_AC_CHV);
			if (r != SC_SUCCESS)
				return r;
			tlen += 2 + p[4];	/* FIXME */
		} else {
			sc_log(card->ctx,  "invalid security condition");
			return SC_ERROR_INTERNAL;
		}
		p   += tlen;
		datalength -= tlen;
	}

	return SC_SUCCESS;