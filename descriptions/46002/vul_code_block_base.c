struct sc_file *file;
	const char *_template = NULL, *desc = NULL;
	unsigned int structure = 0xFFFFFFFF;

	SC_FUNC_CALLED(card->ctx, SC_LOG_DEBUG_VERBOSE);
	sc_log(card->ctx,  "type %X; num %i\n", type,
		 num);
	while (1) {
		switch (type) {
		case SC_PKCS15_TYPE_PRKEY_EC:
			desc = "RSA private key";
			_template = "private-key";
			structure = SC_CARDCTL_OBERTHUR_KEY_EC_CRT;
			break;
		case SC_PKCS15_TYPE_PUBKEY_EC:
			desc = "RSA public key";
			_template = "public-key";
			structure = SC_CARDCTL_OBERTHUR_KEY_EC_PUBLIC;
			break;
		case SC_PKCS15_TYPE_PRKEY_RSA:
			desc = "RSA private key";
			_template = "private-key";
			structure = SC_CARDCTL_OBERTHUR_KEY_RSA_CRT;
			break;
		case SC_PKCS15_TYPE_PUBKEY_RSA:
			desc = "RSA public key";
			_template = "public-key";
			structure = SC_CARDCTL_OBERTHUR_KEY_RSA_PUBLIC;
			break;
		case SC_PKCS15_TYPE_PRKEY:
			desc = "extractable private key";
			_template = "extractable-key";
			break;
		case SC_PKCS15_TYPE_CERT:
			desc = "certificate";
			_template = "certificate";
			break;
		case SC_PKCS15_TYPE_DATA_OBJECT:
			desc = "data object";
			_template = "data";
			break;
		}
		if (_template)
			break;
		/* If this is a specific type such as
		 * SC_PKCS15_TYPE_CERT_FOOBAR, fall back to
		 * the generic class (SC_PKCS15_TYPE_CERT)
		 */
		if (!(type & ~SC_PKCS15_TYPE_CLASS_MASK)) {
			sc_log(card->ctx, 
				 "File type %X not supported by card driver",
				 type);
			return SC_ERROR_INVALID_ARGUMENTS;
		}
		type &= SC_PKCS15_TYPE_CLASS_MASK;
	}

	sc_log(card->ctx,  "template %s; num %i\n",
		 _template, num);
	if (sc_profile_get_file(profile, _template, &file) < 0) {
		sc_log(card->ctx, 
			 "Profile doesn't define %s template '%s'\n", desc,
			 _template);
		return SC_ERROR_NOT_SUPPORTED;
	}
