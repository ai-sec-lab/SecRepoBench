static int
iasecc_parse_acls(struct sc_card *card, struct iasecc_sdo_docp *docp, int flags)
{
	struct sc_context *ctx = card->ctx;
	struct iasecc_extended_tlv *acls = &docp->acls_contact;
	// <MASK>

	sc_log(ctx, "iasecc_parse_docp() SCBs %02X:%02X:%02X:%02X:%02X:%02X:%02X",
			docp->scbs[0],docp->scbs[1],docp->scbs[2],docp->scbs[3],
			docp->scbs[4],docp->scbs[5],docp->scbs[6]);
	LOG_FUNC_RETURN(ctx, SC_SUCCESS);
}