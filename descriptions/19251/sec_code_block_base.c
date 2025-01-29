case CKO_PRIVATE_KEY:
			r = coolkey_get_attribute_ulong(card, &coolkey_obj, CKA_KEY_TYPE, &key_type);
			/* default to CKK_RSA */
			if (r == SC_ERROR_DATA_OBJECT_NOT_FOUND) {
				key_type = CKK_RSA;
				r = SC_SUCCESS;
			}
			if (r < 0) {
				goto fail;
			}
			/* set the info values */
			obj_info = &prkey_info;
			memset(&prkey_info, 0, sizeof(prkey_info));
			coolkey_get_id(card, &coolkey_obj, &prkey_info.id);
			prkey_info.path = coolkey_obj.path;
			prkey_info.key_reference = coolkey_obj.id;
			prkey_info.native = 1;
			coolkey_get_usage(card, &coolkey_obj, &prkey_info.usage);
			coolkey_get_access(card, &coolkey_obj, &prkey_info.access_flags);
			key = coolkey_get_public_key(p15card, &coolkey_obj, key_type);
			if (key_type == CKK_RSA) {
				obj_type = SC_PKCS15_TYPE_PRKEY_RSA;
				if (key) {
					prkey_info.modulus_length =  key->u.rsa.modulus.len*8;
				}
			} else if (key_type == CKK_EC) {
				obj_type = SC_PKCS15_TYPE_PRKEY_EC;
				if (key) {
					prkey_info.field_length =  key->u.ec.params.field_length;
				}
			} else {
				goto fail;
			}
			break;

		case CKO_PUBLIC_KEY:
			r = coolkey_get_attribute_ulong(card, &coolkey_obj, CKA_KEY_TYPE, &key_type);
			/* default to CKK_RSA */
			if (r == SC_ERROR_DATA_OBJECT_NOT_FOUND) {
				key_type = CKK_RSA;
				r = SC_SUCCESS;
			}
			if (r < 0) {
				goto fail;
			}
			key = coolkey_get_public_key(p15card, &coolkey_obj, key_type);
			if (key == NULL) {
				goto fail;
			}
			/* set the info values */
			obj_info = &pubkey_info;
			memset(&pubkey_info, 0, sizeof(pubkey_info));
			r = sc_pkcs15_encode_pubkey_as_spki(card->ctx, key, &pubkey_info.direct.spki.value,
																			&pubkey_info.direct.spki.len);
			if (r < 0)
				goto fail;
			coolkey_get_id(card, &coolkey_obj, &pubkey_info.id);
			pubkey_info.path = coolkey_obj.path;
			pubkey_info.native        = 1;
			pubkey_info.key_reference = coolkey_obj.id;
			coolkey_get_usage(card, &coolkey_obj, &pubkey_info.usage);
			coolkey_get_access(card, &coolkey_obj, &pubkey_info.access_flags);
			if (key_type == CKK_RSA) {
				obj_type = SC_PKCS15_TYPE_PUBKEY_RSA;
				pubkey_info.modulus_length =  key->u.rsa.modulus.len*8;
			} else if (key_type == CKK_EC) {
				obj_type = SC_PKCS15_TYPE_PUBKEY_EC;
				pubkey_info.field_length =  key->u.ec.params.field_length;
			} else {
				goto fail;
			}
			/* set the obj values */
			obj_obj.emulated = key;
			key = NULL;
			break;

		case CKO_CERTIFICATE:
			obj_info = &cert_info;
			memset(&cert_info, 0, sizeof(cert_info));
			coolkey_get_id(card, &coolkey_obj, &cert_info.id);
			cert_info.path = coolkey_obj.path;
			obj_type = SC_PKCS15_TYPE_CERT_X509;

			/* following will find the cached cert in cert_info */
			r = coolkey_get_certificate(card, &coolkey_obj, &cert_info.value);
			if (r < 0) {
				goto fail;
			}
			break;


		default:
			/* no other recognized types which are stored 'on card' */
			sc_log(card->ctx, "Unknown object type %lu, skipping", obj_class);
			continue;