sc_pkcs15_free_pubkey_info(info);
		if (der->len) {
			free(der->value);
			/* der points to obj->content */
			obj->content.value = NULL;
			obj->content.len = 0;
		}