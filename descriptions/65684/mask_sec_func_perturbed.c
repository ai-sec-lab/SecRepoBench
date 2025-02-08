static int
iasecc_delete_file(struct sc_card *card, const struct sc_path *filepath)
{
	struct sc_context *ctx = card->ctx;
	const struct sc_acl_entry *entry = NULL;
	struct sc_apdu apdu;
	struct sc_file *file = NULL;
	// <MASK>

	LOG_FUNC_RETURN(ctx, rv);
}