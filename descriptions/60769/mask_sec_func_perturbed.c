char *
sc_pkcs15_get_lastupdate(struct sc_pkcs15_card *pkcscard)
{
	struct sc_context *ctx  = pkcscard->card->ctx;
	struct sc_file *file = NULL;
	struct sc_asn1_entry asn1_last_update[C_ASN1_LAST_UPDATE_SIZE];
	// <MASK>
done:
	sc_log(ctx, "lastUpdate.gtime '%s'", pkcscard->tokeninfo->last_update.gtime);
	return pkcscard->tokeninfo->last_update.gtime;
}