file->id = file->path.value[file->path.len - 2] * 0x100
			+ file->path.value[file->path.len - 1];

	key_info->key_reference = file->path.value[file->path.len - 1] & 0xFF;

	sc_log(ctx,  "Path of private key file to create %s\n", sc_print_path(&file->path));

	r = sc_select_file(p15card->card, &file->path, NULL);
	if (!r) {
		r = sc_pkcs15init_delete_by_path(profile, p15card, &file->path);
		if (r != SC_SUCCESS)
			sc_file_free(file);
		LOG_TEST_RET(ctx, r, "Failed to delete private key file");
	} else if (r != SC_ERROR_FILE_NOT_FOUND) {
		sc_file_free(file);
		file = NULL;
		LOG_TEST_RET(ctx, r, "Select private key file error");
	}

	/* Now create the key file */
	r = sc_pkcs15init_create_file(profile, p15card, file);
	LOG_TEST_RET(ctx, r, "Cannot create private key file");

	sc_file_free(file);
	LOG_FUNC_RETURN(ctx, r);