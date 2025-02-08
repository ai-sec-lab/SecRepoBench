static int asepcos_parse_sec_attr(sc_card_t *card, sc_file_t *file, const u8 *buf,
	size_t len)
{
	const u8 *p = buf;

	while (len > 0) {
		unsigned int amode, totallen = 3;
		if (len < 5 || p[0] != 0x80 || p[1] != 0x01) {
			sc_log(card->ctx,  "invalid access mode encoding");
			return SC_ERROR_INTERNAL;
		}
		amode = p[2];
		if (p[3] == 0x90 && p[4] == 0x00) {
			int r = set_sec_attr(file, amode, 0, SC_AC_NONE);
			if (r != SC_SUCCESS) 
				return r;
			totallen += 2;
		} else if (p[3] == 0x97 && p[4] == 0x00) {
			int r = set_sec_attr(file, amode, 0, SC_AC_NEVER);
			if (r != SC_SUCCESS) 
				return r;
			totallen += 2;
		} else if (p[3] == 0xA0 && len >= 5U + p[4]) {
			if (len < 6) {
				sc_log(card->ctx,  "invalid access mode encoding");
				return SC_ERROR_INTERNAL;
			}
			/* TODO: support OR expressions */
			int r = set_sec_attr(file, amode, p[5], SC_AC_CHV);
			if (r != SC_SUCCESS)
				return r;
			totallen += 2 + p[4]; /* FIXME */
		} else if (p[3] == 0xAF && len >= 5U + p[4]) {
			if (len < 6) {
				sc_log(card->ctx,  "invalid access mode encoding");
				return SC_ERROR_INTERNAL;
			}
			/* TODO: support AND expressions */
			int r = set_sec_attr(file, amode, p[5], SC_AC_CHV);
			if (r != SC_SUCCESS)
				return r;
			totallen += 2 + p[4];	/* FIXME */
		} else {
			sc_log(card->ctx,  "invalid security condition");
			return SC_ERROR_INTERNAL;
		}
		p   += totallen;
		len -= totallen;
	}

	return SC_SUCCESS;
}