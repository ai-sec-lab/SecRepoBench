static int
cosm_create_key(struct sc_profile *profile, struct sc_pkcs15_card *p15card,
		struct sc_pkcs15_object *object)
{
	struct sc_context *ctx = p15card->card->ctx;
	struct sc_pkcs15_prkey_info *key_info = (struct sc_pkcs15_prkey_info *)object->data;
	struct sc_file *file = NULL;
	int rv = 0;

	SC_FUNC_CALLED(ctx, SC_LOG_DEBUG_VERBOSE);
	if (object->type != SC_PKCS15_TYPE_PRKEY_RSA)
		LOG_TEST_RET(ctx, SC_ERROR_NOT_SUPPORTED, "Create key failed: RSA only supported");

	// <MASK>
}