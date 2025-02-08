if (key_info->path.len < 2)
		LOG_TEST_RET(ctx, SC_ERROR_OBJECT_NOT_VALID, "The path needs to be at least to bytes long");

	sc_log(ctx,  "create private key ID:%s",  sc_pkcs15_print_id(&key_info->id));
	/* Here, the path of private key file should be defined.
	 * Nevertheless, we need to instantiate private key to get the ACLs. */
	rv = cosm_new_file(userprofile, p15card->card, SC_PKCS15_TYPE_PRKEY_RSA, key_info->key_reference, &file);
	LOG_TEST_RET(ctx, rv, "Cannot create key: failed to allocate new key object");

	file->size = key_info->modulus_length;
	memcpy(&file->path, &key_info->path, sizeof(file->path));
	file->id = file->path.value[file->path.len - 2] * 0x100
				+ file->path.value[file->path.len - 1];

	sc_log(ctx,  "Path of private key file to create %s", sc_print_path(&file->path));

	rv = sc_select_file(p15card->card, &file->path, NULL);
	if (rv == 0)   {
		rv = cosm_delete_file(p15card, userprofile, file);
		SC_TEST_GOTO_ERR(ctx, SC_LOG_DEBUG_VERBOSE, rv, "Failed to delete private key file");
	}
	else if (rv != SC_ERROR_FILE_NOT_FOUND)    {
		SC_TEST_GOTO_ERR(ctx, SC_LOG_DEBUG_VERBOSE, rv, "Select private key file error");
	}

	rv = sc_pkcs15init_create_file(userprofile, p15card, file);
	SC_TEST_GOTO_ERR(ctx, SC_LOG_DEBUG_VERBOSE, rv, "Failed to create private key file");

	key_info->key_reference = file->path.value[file->path.len - 1];

err:
	sc_file_free(file);

	LOG_FUNC_RETURN(ctx, rv);