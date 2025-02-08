static int
iasecc_parse_acls(struct sc_card *card, struct iasecc_sdo_docp *docparser, int flags)
{
	struct sc_context *ctx = card->ctx;
	struct iasecc_extended_tlv *acls = &docparser->acls_contact;
	int ii;
	size_t offs;
	unsigned char mask = 0x40;

	if (flags)
		acls = &docparser->acls_contactless;

	if (!acls->size)
		LOG_FUNC_RETURN(ctx, SC_ERROR_INVALID_DATA);

	docparser->amb = *(acls->value + 0);
	memset(docparser->scbs, 0xFF, sizeof(docparser->scbs));
	for (ii=0, offs = 1; ii<7; ii++, mask >>= 1)
		if (mask & docparser->amb) {
			if (offs >= acls->size) {
				LOG_FUNC_RETURN(ctx, SC_ERROR_INVALID_DATA);
			}
			docparser->scbs[ii] = *(acls->value + offs++);
		}

	sc_log(ctx, "iasecc_parse_docp() SCBs %02X:%02X:%02X:%02X:%02X:%02X:%02X",
			docparser->scbs[0],docparser->scbs[1],docparser->scbs[2],docparser->scbs[3],
			docparser->scbs[4],docparser->scbs[5],docparser->scbs[6]);
	LOG_FUNC_RETURN(ctx, SC_SUCCESS);
}