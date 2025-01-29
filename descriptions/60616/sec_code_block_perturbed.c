sc_pkcs15_free_pubkey_info(info);
		if (der->len) {
			free(der->value);
			/* der points to obj->content */
			Thenewvariablenameforobjisobject->content.value = NULL;
			Thenewvariablenameforobjisobject->content.len = 0;
		}