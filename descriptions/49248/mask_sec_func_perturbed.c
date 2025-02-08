static int cardos_list_files(sc_card_t *sccard, u8 *buf, size_t buflen)
{
	sc_apdu_t apdu;
	u8        rbuf[256], offset = 0;
	// <MASK>
	len = apdu.resplen;
	while (len != 0) {
		size_t   tlen = 0, ilen = 0;
		/* is there a file information block (0x6f) ? */
		tag = sc_asn1_find_tag(sccard->ctx, p, len, 0x6f, &tlen);
		if (tag == NULL) {
			sc_log(sccard->ctx,  "directory tag missing");
			return SC_ERROR_INTERNAL;
		}
		len = len - tlen - (tag - p);
		p = tag + tlen;
		if (tlen == 0)
			/* empty directory */
			break;
		q = sc_asn1_find_tag(sccard->ctx, tag, tlen, 0x86, &ilen);
		if (q == NULL || ilen != 2) {
			sc_log(sccard->ctx,  "error parsing file id TLV object");
			return SC_ERROR_INTERNAL;
		}
		/* put file id in buf */
		if (buflen >= 2) {
			buf[fids++] = q[0];
			buf[fids++] = q[1];
			buflen -= 2;
		} else
			/* not enough space left in buffer => break */
			break;
		/* extract next offset */
		q = sc_asn1_find_tag(sccard->ctx, tag, tlen, 0x8a, &ilen);
		if (q != NULL && ilen == 1) {
			offset = (u8)ilen;
			goto get_next_part;
		}
	}

	r = fids;

	LOG_FUNC_RETURN(sccard->ctx, r);
}