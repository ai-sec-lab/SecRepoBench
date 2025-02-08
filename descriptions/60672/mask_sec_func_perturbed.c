static int
setcos_create_key(sc_profile_t *profile, sc_pkcs15_card_t *p15card,
		struct sc_pkcs15_object *objectkey)
{
	struct sc_context *ctx = p15card->card->ctx;
	struct sc_pkcs15_prkey_info *key_info = (struct sc_pkcs15_prkey_info *)objectkey->data;
	struct sc_file *file = NULL;
	int keybits = key_info->modulus_length, r;

	SC_FUNC_CALLED(ctx, SC_LOG_DEBUG_VERBOSE);
	if (objectkey->type != SC_PKCS15_TYPE_PRKEY_RSA)
		LOG_TEST_RET(ctx, SC_ERROR_NOT_SUPPORTED, "Create key failed: RSA only supported");

	/* Parameter check */
	if ((keybits < 512) || (keybits > 1024) || (keybits & 0x7))
		LOG_TEST_RET(ctx, SC_ERROR_INVALID_ARGUMENTS, "Invalid key length");

	sc_log(ctx,  "create private key ID:%s\n",  sc_pkcs15_print_id(&key_info->id));

	/* Get the private key file */
	r = setcos_new_file(profile, p15card->card, SC_PKCS15_TYPE_PRKEY_RSA, key_info->key_reference, &file);
	LOG_TEST_RET(ctx, r, "Cannot get new private key file");

	/* Take enough room for a 1024 bit key */
	if (file->size < 512)
		file->size = 512;

	/* Replace the path of instantiated key template by the path from the object data. */
	memcpy(&file->path, &key_info->path, sizeof(file->path));
	// <MASK>
}