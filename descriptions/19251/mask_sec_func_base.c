static int sc_pkcs15emu_coolkey_init(sc_pkcs15_card_t *p15card)
{
	static const pindata pins[] = {
		{ "1", NULL, "", 0x00,
		  SC_PKCS15_PIN_TYPE_ASCII_NUMERIC,
		  32, 4, 32,
		  SC_PKCS15_PIN_FLAG_INITIALIZED,
		  -1, 0xFF,
		  SC_PKCS15_CO_FLAG_PRIVATE },
		{ NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	};

	/*
	 * The size of the key or the algid is not really known
	 * but can be derived from the certificates.
	 * the cert, pubkey and privkey are a set.
	 * Key usages bits taken from certificate key usage extension.
	 */

	int    r, i;
	sc_card_t *card = p15card->card;
	sc_serial_number_t serial;
	int count;
	struct sc_pkcs15_object *obj;

	SC_FUNC_CALLED(card->ctx, SC_LOG_DEBUG_VERBOSE);

	memset(&serial, 0, sizeof(serial));

	/* coolkey caches a nonce once it logs in, don't keep the pin around. The card will
	 * stay logged in until it's been pulled from the reader, in which case you want to reauthenticate
	 * anyway */
	p15card->opts.use_pin_cache = 0;


	/* get the token info from the card */
	r = sc_card_ctl(card, SC_CARDCTL_COOLKEY_GET_TOKEN_INFO, p15card->tokeninfo);
	if (r < 0) {
		/* put some defaults in if we didn't succeed */
		p15card->tokeninfo->label = strdup("Coolkey");
		p15card->tokeninfo->manufacturer_id = strdup("Unknown");
		p15card->tokeninfo->serial_number = strdup("00000000");
	}

	/* set pins */
	sc_log(card->ctx,  "Coolkey adding pins...");
	for (i = 0; pins[i].id; i++) {
		struct sc_pkcs15_auth_info pin_info;
		struct sc_pkcs15_object   pin_obj;
		const char * label;

		memset(&pin_info, 0, sizeof(pin_info));
		memset(&pin_obj,  0, sizeof(pin_obj));

		pin_info.auth_type = SC_PKCS15_PIN_AUTH_TYPE_PIN;
		sc_pkcs15_format_id(pins[i].id, &pin_info.auth_id);
		pin_info.attrs.pin.reference     = pins[i].ref;
		pin_info.attrs.pin.flags         = pins[i].flags;
		pin_info.attrs.pin.type          = pins[i].type;
		pin_info.attrs.pin.min_length    = pins[i].minlen;
		pin_info.attrs.pin.stored_length = pins[i].storedlen;
		pin_info.attrs.pin.max_length    = pins[i].maxlen;
		pin_info.attrs.pin.pad_char      = pins[i].pad_char;
		sc_format_path(pins[i].path, &pin_info.path);
		pin_info.tries_left    = -1;

		label = pins[i].label? pins[i].label : p15card->tokeninfo->label;
		sc_log(card->ctx,  "Coolkey Adding pin %d label=%s",i, label);
		strncpy(pin_obj.label, label, SC_PKCS15_MAX_LABEL_SIZE - 1);
		pin_obj.flags = pins[i].obj_flags;

		r = sc_pkcs15emu_add_pin_obj(p15card, &pin_obj, &pin_info);
		if (r < 0)
			LOG_FUNC_RETURN(card->ctx, r);
	}

	/* set other objects */
	r = (card->ops->card_ctl)(card, SC_CARDCTL_COOLKEY_INIT_GET_OBJECTS, &count);
	LOG_TEST_RET(card->ctx, r, "Can not initiate objects.");

	for (i = 0; i < count; i++) {
		struct sc_cardctl_coolkey_object     coolkey_obj;
		struct sc_pkcs15_object    obj_obj;
		struct sc_pkcs15_cert_info cert_info;
		struct sc_pkcs15_pubkey_info pubkey_info;
		struct sc_pkcs15_prkey_info prkey_info;
		sc_pkcs15_pubkey_t *key = NULL;
		void *obj_info = NULL;
		int obj_type = 0;
		CK_KEY_TYPE key_type;
		CK_OBJECT_CLASS obj_class;
		size_t len;

		r = (card->ops->card_ctl)(card, SC_CARDCTL_COOLKEY_GET_NEXT_OBJECT, &coolkey_obj);
		if (r < 0)
			LOG_FUNC_RETURN(card->ctx, r);


		memset(&obj_obj, 0, sizeof(obj_obj));
		/* coolkey applets have label only on the certificates,
		 * but we should copy it also to the keys matching the same ID */
		coolkey_get_attribute_bytes(card, &coolkey_obj, CKA_LABEL, (u8 *)obj_obj.label, &len, sizeof(obj_obj.label));
		coolkey_get_flags(card, &coolkey_obj, &obj_obj.flags);
		if (obj_obj.flags & SC_PKCS15_CO_FLAG_PRIVATE) {
			sc_pkcs15_format_id(pins[0].id, &obj_obj.auth_id);
		}

		r = coolkey_get_attribute_ulong(card, &coolkey_obj, CKA_CLASS, &obj_class);
		if (r < 0) {
			goto fail;
		}
		switch (obj_class) {
		// <MASK>
		}
		if (obj_info == NULL) {
			continue;
		}

		r = sc_pkcs15emu_object_add(p15card, obj_type, &obj_obj, obj_info);
		if (r != SC_SUCCESS)
			sc_log(card->ctx, "sc_pkcs15emu_object_add() returned %d", r);
fail:
		if (key) { sc_pkcs15_free_pubkey(key); }

	}
	r = (card->ops->card_ctl)(card, SC_CARDCTL_COOLKEY_FINAL_GET_OBJECTS, &count);
	LOG_TEST_RET(card->ctx, r, "Can not finalize objects.");

	/* Iterate over all the created objects and fill missing labels */
	for (obj = p15card->obj_list; obj != NULL; obj = obj->next) {
		struct sc_pkcs15_id *id = NULL;
		struct sc_pkcs15_object *cert_object;

		/* label non-empty -- do not overwrite */
		if (obj->label[0] != '\0')
			continue;

		switch (obj->type & SC_PKCS15_TYPE_CLASS_MASK) {
		case SC_PKCS15_TYPE_PUBKEY:
			id = &((struct sc_pkcs15_pubkey_info *)obj->data)->id;
			break;
		case SC_PKCS15_TYPE_PRKEY:
			id = &((struct sc_pkcs15_prkey_info *)obj->data)->id;
			break;
		default:
			/* We do not care about other objects */
			continue;
		}
		r = sc_pkcs15_find_cert_by_id(p15card, id, &cert_object);
		if (r != 0)
			continue;

		sc_log(card->ctx, "Copy label \"%s\" from cert to key object",
			cert_object->label);
		memcpy(obj->label, cert_object->label, SC_PKCS15_MAX_LABEL_SIZE);
	}

	LOG_FUNC_RETURN(card->ctx, SC_SUCCESS);
}