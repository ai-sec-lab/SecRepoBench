tag = sc_asn1_find_tag(card->ctx, body, bodylen, 0x71, &taglen);
		/* 800-72-1 not clear if this is 80 or 01 Sent comment to NIST for 800-72-2 */
		/* 800-73-3 says it is 01, keep dual test so old cards still work */
		if (tag && (((*tag) & 0x80) || ((*tag) & 0x01)))
			compressed = 1;

		tag = sc_asn1_find_tag(card->ctx, body, bodylen, 0x70, &taglen);