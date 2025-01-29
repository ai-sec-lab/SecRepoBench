memset(gostr3410_params, 0, sizeof(gostr3410_params));

	r = sc_asn1_decode(ctx, asn1_pubkey, *buf, *buflen, buf, buflen);
	if (r == SC_ERROR_ASN1_END_OF_CONTENTS)
		goto err;
	LOG_TEST_GOTO_ERR(ctx, r, "ASN.1 decoding failed");
	if (asn1_pubkey_choice[0].flags & SC_ASN1_PRESENT) {
		obj->type = SC_PKCS15_TYPE_PUBKEY_RSA;
	} else if (asn1_pubkey_choice[2].flags & SC_ASN1_PRESENT) {
		obj->type = SC_PKCS15_TYPE_PUBKEY_GOSTR3410;
		assert(info->modulus_length == 0);
		info->modulus_length = SC_PKCS15_GOSTR3410_KEYSIZE;
		assert(info->params.len == 0);
		info->params.len = sizeof(struct sc_pkcs15_keyinfo_gostparams);
		info->params.data = malloc(info->params.len);
		if (info->params.data == NULL) {
			r = SC_ERROR_OUT_OF_MEMORY;
			goto err;
		}
		assert(sizeof(*keyinfo_gostparams) == info->params.len);
		keyinfo_gostparams = info->params.data;
		keyinfo_gostparams->gostr3410 = (unsigned int)gostr3410_params[0];
		keyinfo_gostparams->gostr3411 = (unsigned int)gostr3410_params[1];
		keyinfo_gostparams->gost28147 = (unsigned int)gostr3410_params[2];
	}
	else if (asn1_pubkey_choice[3].flags & SC_ASN1_PRESENT) {
		obj->type = SC_PKCS15_TYPE_PUBKEY_EC;
	}
	else {
		obj->type = SC_PKCS15_TYPE_PUBKEY_DSA;
	}