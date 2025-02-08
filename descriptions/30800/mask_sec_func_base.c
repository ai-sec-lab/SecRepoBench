static int
iasecc_process_fci(struct sc_card *card, struct sc_file *file,
		 const unsigned char *buf, size_t buflen)
{
	struct sc_context *ctx = card->ctx;
	size_t taglen;
	int rv, ii, offs;
	const unsigned char *acls = NULL, *tag = NULL;
	unsigned char mask;
	unsigned char ops_DF[7] = {
		SC_AC_OP_DELETE, 0xFF, SC_AC_OP_ACTIVATE, SC_AC_OP_DEACTIVATE, 0xFF, SC_AC_OP_CREATE, 0xFF
	};
	unsigned char ops_EF[7] = {
		SC_AC_OP_DELETE, 0xFF, SC_AC_OP_ACTIVATE, SC_AC_OP_DEACTIVATE, 0xFF, SC_AC_OP_UPDATE, SC_AC_OP_READ
	};

	LOG_FUNC_CALLED(ctx);

	tag = sc_asn1_find_tag(ctx,  buf, buflen, 0x6F, &taglen);
	sc_log(ctx, "processing FCI: 0x6F tag %p", tag);
	if (tag != NULL) {
		sc_log(ctx, "  FCP length %"SC_FORMAT_LEN_SIZE_T"u", taglen);
		buf = tag;
		buflen = taglen;
	}

	tag = sc_asn1_find_tag(ctx,  buf, buflen, 0x62, &taglen);
	sc_log(ctx, "processing FCI: 0x62 tag %p", tag);
	if (tag != NULL) {
		sc_log(ctx, "  FCP length %"SC_FORMAT_LEN_SIZE_T"u", taglen);
		buf = tag;
		buflen = taglen;
	}

	rv = iso_ops->process_fci(card, file, buf, buflen);
	LOG_TEST_RET(ctx, rv, "ISO parse FCI failed");
/*
	Gemalto:  6F 19 80 02 02 ED 82 01 01 83 02 B0 01 88 00	8C 07 7B 17 17 17 17 17 00 8A 01 05 90 00
	Sagem:    6F 17 62 15 80 02 00 7D 82 01 01                   8C 02 01 00 83 02 2F 00 88 01 F0 8A 01 05 90 00
	Oberthur: 62 1B 80 02 05 DC 82 01 01 83 02 B0 01 88 00 A1 09 8C 07 7B 17 FF 17 17 17 00 8A 01 05 90 00
*/

	sc_log(ctx, "iasecc_process_fci() type %i; let's parse file ACLs", file->type);
	tag = sc_asn1_find_tag(ctx, buf, buflen, IASECC_DOCP_TAG_ACLS, &taglen);
	// <MASK>
}