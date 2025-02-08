static int asepcos_parse_sec_attr(sc_card_t *card, sc_file_t *file, const u8 *buf,
	size_t length)
{
	const u8 *p = buf;

	while (length > 0) {
		unsigned int amode, tlen = 3;
		if (length < 5 || p[0] != 0x80 || p[1] != 0x01) {
			sc_log(card->ctx,  "invalid access mode encoding");
			return SC_ERROR_INTERNAL;
		}
		amode = p[2];
		if (p[3] == 0x90 && p[4] == 0x00) {
			int r = set_sec_attr(file, amode, 0, SC_AC_NONE);
			if (r != SC_SUCCESS) 
				return r;
			tlen += 2;
		} else // <MASK>
		p   += tlen;
		length -= tlen;
	}

	return SC_SUCCESS;
}