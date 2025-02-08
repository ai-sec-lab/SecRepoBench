int jj;
		unsigned char resp[256];
		size_t resp_len, encodedlen;
		struct sc_pkcs15_pubkey_rsa key;

		resp_len = sizeof(resp);
		rv = auth_read_component(card, SC_CARDCTL_OBERTHUR_KEY_RSA_PUBLIC,
				2, resp, resp_len);
		LOG_TEST_RET(card->ctx, rv, "read component failed");

		for (jj=0; jj<rv && *(resp+jj)==0; jj++)
			;

		if (rv - jj == 0)
			return SC_ERROR_INVALID_DATA;
		bn[0].data = calloc(1, rv - jj);
		if (!bn[0].data) {
			rv = SC_ERROR_OUT_OF_MEMORY;
			goto err;
		}
		bn[0].len = rv - jj;
		memcpy(bn[0].data, resp + jj, rv - jj);

		rv = auth_read_component(card, SC_CARDCTL_OBERTHUR_KEY_RSA_PUBLIC,
				1, resp, resp_len);
		LOG_TEST_GOTO_ERR(card->ctx, rv, "Cannot read RSA public key component");

		bn[1].data = calloc(1, rv);
		if (!bn[1].data) {
			rv = SC_ERROR_OUT_OF_MEMORY;
			goto err;
		}
		bn[1].len = rv;
		memcpy(bn[1].data, resp, rv);

		key.exponent = bn[0];
		key.modulus = bn[1];

		if (sc_pkcs15_encode_pubkey_rsa(card->ctx, &key, &out, &encodedlen)) {
			rv = SC_ERROR_INVALID_ASN1_OBJECT;
			LOG_TEST_GOTO_ERR(card->ctx, rv, "cannot encode RSA public key");
		}
		else {
			rv  = encodedlen - offset > count ? count : encodedlen - offset;
			memcpy(buf, out + offset, rv);

			sc_log_hex(card->ctx, "write_publickey", buf, rv);
		}