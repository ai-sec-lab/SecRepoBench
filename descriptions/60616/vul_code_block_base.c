sc_pkcs15_free_pubkey_info(info);
		if (der->len)
			free(der->value);