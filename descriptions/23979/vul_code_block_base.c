
	SC_FUNC_CALLED(card->ctx, SC_LOG_DEBUG_VERBOSE);
	sc_log(card->ctx, "datalen=%"SC_FORMAT_LEN_SIZE_T"u outlen=%"SC_FORMAT_LEN_SIZE_T"u\n",
		datalen, max_out_len);

	if (priv->key_id > 0xff) {
		r = SC_ERROR_NO_DEFAULT_KEY;
		goto done;
	}
	key_number = priv->key_id;

	memset(&params, 0, sizeof(params));
	params.init.mode = COOLKEY_CRYPT_MODE_RSA_NO_PAD;
	params.init.direction = COOLKEY_CRYPT_DIRECTION_ENCRYPT; /* for no pad, direction is irrelevant */

	/* send the data to the card if necessary */
	if (datalen > MAX_COMPUTE_BUF) {
		/* We need to write data to special object on the card as it does not safely fit APDU */
		u8 len_buf[2];

		params.init.location = COOLKEY_CRYPT_LOCATION_DL_OBJECT;

		params_len = sizeof(params.init);

		*crypt_out_p = NULL;
		crypt_out_len_p = 0;

		ushort2bebytes(len_buf, datalen);

		r = coolkey_write_object(card, COOLKEY_DL_OBJECT_ID, 0, len_buf, sizeof(len_buf),
					priv->nonce, sizeof(priv->nonce));
		if (r < 0) {
			goto done;
		}

		r = coolkey_write_object(card, COOLKEY_DL_OBJECT_ID, 2, data, datalen, priv->nonce, sizeof(priv->nonce));
		if (r < 0) {
			goto done;
		}
		ushort2bebytes(params.init.buf_len, 0);
	} else {
		/* The data fits in APDU. Copy it to the params object */
		u8 *buf_out;
		size_t buf_len;

		params.init.location = COOLKEY_CRYPT_LOCATION_APDU;

		params_len = sizeof(params.init) + datalen;

		buf_out = &buf[0];
		crypt_out_p = &buf_out;
		buf_len = sizeof(buf);
		crypt_out_len_p = buf_len;

		ushort2bebytes(params.init.buf_len, datalen);
		memcpy(params.buf, data, datalen);
	}