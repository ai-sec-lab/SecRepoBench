int sc_pkcs15_decode_pukdf_entry(struct sc_pkcs15_card *p15card,
		struct sc_pkcs15_object *obj,
		const u8 ** buf, size_t *buflen)
{
	struct sc_context *ctx = p15card->card->ctx;
	struct sc_pkcs15_pubkey_info *info;
	int r, gostr3410_params[3];
	struct sc_pkcs15_keyinfo_gostparams *keyinfo_gostparams;
	size_t usage_len, af_len;
	struct sc_pkcs15_der *der = &obj->content;
	struct sc_asn1_entry asn1_com_key_attr[C_ASN1_COM_KEY_ATTR_SIZE];
	struct sc_asn1_entry asn1_com_pubkey_attr[C_ASN1_COM_PUBKEY_ATTR_SIZE];
	struct sc_asn1_entry asn1_rsakey_value_choice[C_ASN1_RSAKEY_VALUE_CHOICE_SIZE];
	struct sc_asn1_entry asn1_rsakey_attr[C_ASN1_RSAKEY_ATTR_SIZE];
	struct sc_asn1_entry asn1_rsa_type_attr[C_ASN1_RSA_TYPE_ATTR_SIZE];
	struct sc_asn1_entry asn1_eckey_value_choice[C_ASN1_ECKEY_VALUE_CHOICE_SIZE];
	struct sc_asn1_entry asn1_eckey_attr[C_ASN1_ECKEY_ATTR_SIZE];
	struct sc_asn1_entry asn1_ec_type_attr[C_ASN1_EC_TYPE_ATTR_SIZE];
	struct sc_asn1_entry asn1_gostr3410key_attr[C_ASN1_GOST3410KEY_ATTR_SIZE];
	struct sc_asn1_entry asn1_gostr3410_type_attr[C_ASN1_GOST3410_TYPE_ATTR_SIZE];
	struct sc_asn1_entry asn1_pubkey_choice[C_ASN1_PUBKEY_CHOICE_SIZE];
	struct sc_asn1_entry asn1_pubkey[C_ASN1_PUBKEY_SIZE];
	struct sc_asn1_pkcs15_object rsakey_obj = { obj, asn1_com_key_attr,
			asn1_com_pubkey_attr, asn1_rsa_type_attr };
	struct sc_asn1_pkcs15_object eckey_obj = { obj, asn1_com_key_attr,
			asn1_com_pubkey_attr, asn1_ec_type_attr };
	struct sc_asn1_pkcs15_object gostr3410key_obj =  { obj, asn1_com_key_attr,
			asn1_com_pubkey_attr, asn1_gostr3410_type_attr };

	info = calloc(1, sizeof *info);
	if (info == NULL) {
		r = SC_ERROR_OUT_OF_MEMORY;
		goto err;
	}
	usage_len = sizeof(info->usage);
	af_len = sizeof(info->access_flags);

	sc_copy_asn1_entry(c_asn1_pubkey, asn1_pubkey);
	sc_copy_asn1_entry(c_asn1_pubkey_choice, asn1_pubkey_choice);
	sc_copy_asn1_entry(c_asn1_rsa_type_attr, asn1_rsa_type_attr);
	sc_copy_asn1_entry(c_asn1_rsakey_value_choice, asn1_rsakey_value_choice);
	sc_copy_asn1_entry(c_asn1_rsakey_attr, asn1_rsakey_attr);
	sc_copy_asn1_entry(c_asn1_ec_type_attr, asn1_ec_type_attr);
	sc_copy_asn1_entry(c_asn1_eckey_value_choice, asn1_eckey_value_choice);
	sc_copy_asn1_entry(c_asn1_eckey_attr, asn1_eckey_attr);
	sc_copy_asn1_entry(c_asn1_gostr3410_type_attr, asn1_gostr3410_type_attr);
	sc_copy_asn1_entry(c_asn1_gostr3410key_attr, asn1_gostr3410key_attr);
	sc_copy_asn1_entry(c_asn1_com_pubkey_attr, asn1_com_pubkey_attr);
	sc_copy_asn1_entry(c_asn1_com_key_attr, asn1_com_key_attr);

	sc_format_asn1_entry(asn1_com_pubkey_attr + 0, &info->subject.value, &info->subject.len, 0);

	sc_format_asn1_entry(asn1_pubkey_choice + 0, &rsakey_obj, NULL, 0);
	sc_format_asn1_entry(asn1_pubkey_choice + 1, &gostr3410key_obj, NULL, 0);
	sc_format_asn1_entry(asn1_pubkey_choice + 2, &eckey_obj, NULL, 0);

	sc_format_asn1_entry(asn1_rsa_type_attr + 0, asn1_rsakey_attr, NULL, 0);

	sc_format_asn1_entry(asn1_rsakey_value_choice + 0, &info->path, NULL, 0);
	sc_format_asn1_entry(asn1_rsakey_value_choice + 1, &der->value, &der->len, 0);

	sc_format_asn1_entry(asn1_rsakey_attr + 0, asn1_rsakey_value_choice, NULL, 0);
	sc_format_asn1_entry(asn1_rsakey_attr + 1, &info->modulus_length, NULL, 0);

	sc_format_asn1_entry(asn1_ec_type_attr + 0, asn1_eckey_attr, NULL, 0);

	sc_format_asn1_entry(asn1_eckey_value_choice + 0, &info->path, NULL, 0);
	sc_format_asn1_entry(asn1_eckey_value_choice + 1, &der->value, &der->len, 0);

	sc_format_asn1_entry(asn1_eckey_attr + 0, asn1_eckey_value_choice, NULL, 0);

	sc_format_asn1_entry(asn1_gostr3410_type_attr + 0, asn1_gostr3410key_attr, NULL, 0);

	sc_format_asn1_entry(asn1_gostr3410key_attr + 0, &info->path, NULL, 0);
	sc_format_asn1_entry(asn1_gostr3410key_attr + 1, &gostr3410_params[0], NULL, 0);
	sc_format_asn1_entry(asn1_gostr3410key_attr + 2, &gostr3410_params[1], NULL, 0);
	sc_format_asn1_entry(asn1_gostr3410key_attr + 3, &gostr3410_params[2], NULL, 0);

	sc_format_asn1_entry(asn1_com_key_attr + 0, &info->id, NULL, 0);
	sc_format_asn1_entry(asn1_com_key_attr + 1, &info->usage, &usage_len, 0);
	sc_format_asn1_entry(asn1_com_key_attr + 2, &info->native, NULL, 0);
	sc_format_asn1_entry(asn1_com_key_attr + 3, &info->access_flags, &af_len, 0);
	sc_format_asn1_entry(asn1_com_key_attr + 4, &info->key_reference, NULL, 0);

	sc_format_asn1_entry(asn1_pubkey + 0, asn1_pubkey_choice, NULL, 0);

	/* Fill in defaults */
	info->key_reference = -1;
	info->native = 1;
	memset(gostr3410_params, 0, sizeof(gostr3410_params));

	r = sc_asn1_decode(ctx, asn1_pubkey, *buf, *buflen, buf, buflen);
	if (r == SC_ERROR_ASN1_END_OF_CONTENTS)
		goto err;
	LOG_TEST_GOTO_ERR(ctx, r, "ASN.1 decoding failed");
	if (asn1_pubkey_choice[0].flags & SC_ASN1_PRESENT) {
		obj->type = SC_PKCS15_TYPE_PUBKEY_RSA;
	} else if (asn1_pubkey_choice[1].flags & SC_ASN1_PRESENT) {
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
	else if (asn1_pubkey_choice[2].flags & SC_ASN1_PRESENT) {
		obj->type = SC_PKCS15_TYPE_PUBKEY_EC;
	}
	else {
		goto err;
	}

	if (!p15card->app || !p15card->app->ddo.aid.len) {
		if (!p15card->file_app) {
			r = SC_ERROR_INTERNAL;
			goto err;
		}
		r = sc_pkcs15_make_absolute_path(&p15card->file_app->path, &info->path);
		if (r < 0) {
			goto err;
		}
	}
	else   {
		info->path.aid = p15card->app->ddo.aid;
	}
	sc_log(ctx, "PubKey path '%s'", sc_print_path(&info->path));

	/* OpenSC 0.11.4 and older encoded "keyReference" as a negative
	   value. Fixed in 0.11.5 we need to add a hack, so old cards
	   continue to work. */
	if (info->key_reference < -1)
		info->key_reference += 256;

	obj->data = info;
	info = NULL;

	r = sc_pkcs15_decode_pubkey_direct_value(p15card, obj);
	if (r < 0) {
		info = obj->data;
		obj->data = NULL;
	}
	LOG_TEST_GOTO_ERR(ctx, r, "Decode public key direct value failed");

err:
	if (r < 0) {
		sc_pkcs15_free_pubkey_info(info);
		if (der->len) {
			free(der->value);
			/* der points to obj->content */
			obj->content.value = NULL;
			obj->content.len = 0;
		}
	}

	LOG_FUNC_RETURN(ctx, r);
}