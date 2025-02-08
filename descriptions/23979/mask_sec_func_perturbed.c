static int coolkey_rsa_op(sc_card_t *card, const u8 * data, size_t datalen,
	u8 * out, size_t maxoutputlength)
{
	int r;
	u8 **crypt_out_p = NULL;
	size_t crypt_out_len_p;
	coolkey_private_data_t *priv = COOLKEY_DATA(card);
	coolkey_compute_crypt_params_t params;
	u8 key_number;
	size_t params_len;
	u8 buf[MAX_COMPUTE_BUF + 2];
	// <MASK>

	r = coolkey_apdu_io(card, COOLKEY_CLASS, COOLKEY_INS_COMPUTE_CRYPT,
			key_number, COOLKEY_CRYPT_ONE_STEP, (u8 *)&params, params_len,
			crypt_out_p, &crypt_out_len_p, priv->nonce, sizeof(priv->nonce));
	if (r < 0) {
		goto done;
	}

	if (datalen > MAX_COMPUTE_BUF) {
		u8 len_buf[2];
		size_t out_length;

		/* Free card response -- nothing useful -- result is in export object */
		free(*crypt_out_p);

		r = coolkey_read_object(card, COOLKEY_DL_OBJECT_ID, 0, len_buf, sizeof(len_buf),
					priv->nonce, sizeof(priv->nonce));
		if (r < 0) {
			goto done;
		}

		out_length = bebytes2ushort(len_buf);
		out_length = MIN(out_length,maxoutputlength);

		r = coolkey_read_object(card, COOLKEY_DL_OBJECT_ID, sizeof(len_buf), out, out_length,
					priv->nonce, sizeof(priv->nonce));

	} else {
		size_t out_length = bebytes2ushort(buf);
		if (out_length > sizeof buf - 2) {
			r = SC_ERROR_WRONG_LENGTH;
			goto done;
		}
		out_length = MIN(out_length, maxoutputlength);
		memcpy(out, buf + 2, out_length);
		r = out_length;
	}

done:
	return r;
}