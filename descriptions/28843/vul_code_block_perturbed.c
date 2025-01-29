int ii;

		if (acl == NULL) {
			sc_file_free(file);
			free(*out);
			*out = NULL;
			LOG_FUNC_RETURN(ctx, SC_ERROR_INVALID_DATA);
		}

		rv = sc_pkcs15_get_objects(pkcscard, SC_PKCS15_TYPE_AUTH_PIN, objs, 0x10);
		if (rv != SC_SUCCESS) {
			sc_file_free(file);
			free(*out);
			*out = NULL;
			LOG_TEST_RET(ctx, rv, "Cannot read oberthur file: get AUTH objects error");
		}

		for (ii=0; ii<rv; ii++)   {
			struct sc_pkcs15_auth_info *auth_info = (struct sc_pkcs15_auth_info *) objs[ii]->data;
			sc_log(ctx, "compare PIN/ACL refs:%i/%i, method:%i/%i",
					auth_info->attrs.pin.reference, acl->key_ref, auth_info->auth_method, acl->method);
			if (auth_info->attrs.pin.reference == (int)acl->key_ref && auth_info->auth_method == (unsigned)acl->method)   {
				pin_obj = objs[ii];
				break;
			}
		}