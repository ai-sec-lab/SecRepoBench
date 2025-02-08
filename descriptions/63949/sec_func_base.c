static int
iasecc_parse_acls(struct sc_card *card, struct iasecc_sdo_docp *docp, int flags)
{
	struct sc_context *ctx = card->ctx;
	struct iasecc_extended_tlv *acls = &docp->acls_contact;
	int ii;
	size_t offs;
	unsigned char mask = 0x40;

	if (flags)
		acls = &docp->acls_contactless;

	if (!acls->size)
		LOG_FUNC_RETURN(ctx, SC_ERROR_INVALID_DATA);

	docp->amb = *(acls->value + 0);
	memset(docp->scbs, 0xFF, sizeof(docp->scbs));
	for (ii=0, offs = 1; ii<7; ii++, mask >>= 1)
		if (mask & docp->amb) {
			if (offs >= acls->size) {
				LOG_FUNC_RETURN(ctx, SC_ERROR_INVALID_DATA);
			}
			docp->scbs[ii] = *(acls->value + offs++);
		}

	sc_log(ctx, "iasecc_parse_docp() SCBs %02X:%02X:%02X:%02X:%02X:%02X:%02X",
			docp->scbs[0],docp->scbs[1],docp->scbs[2],docp->scbs[3],
			docp->scbs[4],docp->scbs[5],docp->scbs[6]);
	LOG_FUNC_RETURN(ctx, SC_SUCCESS);
}