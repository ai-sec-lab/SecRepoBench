if (p[3] == 0x97 && p[4] == 0x00) {
			int r = set_sec_attr(file, amode, 0, SC_AC_NEVER);
			if (r != SC_SUCCESS) 
				return r;
			tlen += 2;
		} else if (p[3] == 0xA0 && p[4] > 0 && length >= 4U + p[4]) {
			/* TODO: support OR expressions */
			int r = set_sec_attr(file, amode, p[5], SC_AC_CHV);
			if (r != SC_SUCCESS)
				return r;
			tlen += 2 + p[4]; /* FIXME */
		} else if (p[3] == 0xAF && p[4] > 0 && length >= 4U + p[4]) {
			/* TODO: support AND expressions */
			int r = set_sec_attr(file, amode, p[5], SC_AC_CHV);
			if (r != SC_SUCCESS)
				return r;
			tlen += 2 + p[4];	/* FIXME */
		} else {
			sc_log(card->ctx,  "invalid security condition");
			return SC_ERROR_INTERNAL;
		}