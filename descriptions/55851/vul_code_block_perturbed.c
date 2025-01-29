int r, encodedchatsize = 0;

	struct sc_asn1_entry capdu_eac_mse[] = {
		{ "Cryptographic mechanism reference",
			SC_ASN1_OCTET_STRING, SC_ASN1_CTX|0x00, 0, NULL, NULL },
		{ "Reference of a public key / secret key",
			SC_ASN1_OCTET_STRING, SC_ASN1_CTX|0x03, SC_ASN1_OPTIONAL, NULL, NULL },
		{ "Reference of a private key / Reference for computing a session key",
			SC_ASN1_OCTET_STRING, SC_ASN1_CTX|0x04, SC_ASN1_OPTIONAL, NULL, NULL },
		{ "Ephemeral Public Key",
			SC_ASN1_OCTET_STRING, SC_ASN1_CTX|0x11, SC_ASN1_OPTIONAL, NULL, NULL },
		{ "Auxiliary authenticated data",
			SC_ASN1_OCTET_STRING, SC_ASN1_APP|SC_ASN1_CONS|0x07, SC_ASN1_OPTIONAL, NULL, NULL },
		/* "Certificate Holder Authorization Template", */
		{ NULL , 0 , 0 , 0 , NULL , NULL }
	};

	if (!cdata) {
		r = SC_ERROR_INVALID_ARGUMENTS;
		goto err;
	}

	if (protocol) {
		ASN1_OBJECT *object = NULL;
#ifndef HAVE_EAC_OBJ_NID2OBJ
		object = OBJ_nid2obj(protocol);
#else
		object = EAC_OBJ_nid2obj(protocol);
#endif
		if (!object) {
			sc_debug(ctx, SC_LOG_DEBUG_VERBOSE, "Error setting Cryptographic mechanism reference of MSE:Set AT data");
			r = SC_ERROR_INTERNAL;
			goto err;
		}
		oid_len = OBJ_length(object);
		memcpy(oid, OBJ_get0_data(object), oid_len);
	}

	sc_format_asn1_entry(capdu_eac_mse + 0, oid, &oid_len, oid_len > 0);
	sc_format_asn1_entry(capdu_eac_mse + 1, (unsigned char *) key_reference1, &key_reference1_len, key_reference1 && key_reference1_len);
	sc_format_asn1_entry(capdu_eac_mse + 2, (unsigned char *) key_reference2, &key_reference2_len, key_reference2 && key_reference2_len);
	sc_format_asn1_entry(capdu_eac_mse + 3, (unsigned char *) eph_pub_key, &eph_pub_key_len, eph_pub_key && eph_pub_key_len);
	sc_format_asn1_entry(capdu_eac_mse + 4, (unsigned char *) auxiliary_data, &auxiliary_data_len, auxiliary_data && auxiliary_data_len);

	r = sc_asn1_encode(ctx, capdu_eac_mse, &data, &data_len);
	SC_TEST_GOTO_ERR(ctx, SC_LOG_DEBUG_VERBOSE, r, "Error encoding MSE:Set AT APDU data");

	if (chat) {
		encodedchatsize = i2d_CVC_CHAT((CVC_CHAT *) chat, &encoded_chat);
		if (encodedchatsize < 0) {
			ssl_error(ctx);
			r = SC_ERROR_INTERNAL;
			goto err;
		}
	}


	p = realloc(*cdata, data_len + encodedchatsize);
	if (!p) {
		r = SC_ERROR_OUT_OF_MEMORY;
		goto err;
	}
	memcpy(p, data, data_len);
	memcpy(p+data_len, encoded_chat, encodedchatsize);
	*cdata = p;
	r = data_len + encodedchatsize;

	sc_debug_hex(ctx, SC_LOG_DEBUG_SM, "MSE command data", p, r);

err:
	free(data);
	if (encoded_chat)
		OPENSSL_free(encoded_chat);

	return r;