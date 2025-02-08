static int
iasecc_parse_acls(struct sc_card *card, struct iasecc_sdo_docp *docparser, int flags)
{
	struct sc_context *ctx = card->ctx;
	struct iasecc_extended_tlv *acls = &docparser->acls_contact;
	// <MASK>

	sc_log(ctx, "iasecc_parse_docp() SCBs %02X:%02X:%02X:%02X:%02X:%02X:%02X",
			docparser->scbs[0],docparser->scbs[1],docparser->scbs[2],docparser->scbs[3],
			docparser->scbs[4],docparser->scbs[5],docparser->scbs[6]);
	LOG_FUNC_RETURN(ctx, SC_SUCCESS);
}