int r;

	LOG_FUNC_CALLED(cardcontext->ctx);

	if (tname == NULL) {
		LOG_FUNC_RETURN(cardcontext->ctx, SC_ERROR_INVALID_ARGUMENTS);
	}

	if (!priv->tinfo_present) {
		LOG_FUNC_RETURN(cardcontext->ctx, SC_ERROR_NOT_SUPPORTED);
	}

	memcpy(tinfo_path.value, priv->tinfo_df, 2);
	r = iso_ops->select_file(cardcontext, &tinfo_path, &file);
	if (r != SC_SUCCESS || file->size == 0) {
		sc_file_free(file);
		LOG_FUNC_RETURN(cardcontext->ctx, SC_ERROR_NOT_SUPPORTED);
	}

	/* First two bytes lists 0x01, the second indicates length */
	r = iso_ops->read_binary(cardcontext, 0, buf, 2, 0);
	if (r < 2 || buf[1] > file->size) { /* make sure we do not overrun */
		sc_file_free(file);
		LOG_FUNC_RETURN(cardcontext->ctx, r);
	}
	sc_file_free(file);

	*tname = malloc(buf[1]);
	if (*tname == NULL) {
		LOG_FUNC_RETURN(cardcontext->ctx, SC_ERROR_OUT_OF_MEMORY);
	}

	r = iso_ops->read_binary(cardcontext, 2, (unsigned char *)*tname, buf[1], 0);
	if (r < 1) {
		free(*tname);
		LOG_FUNC_RETURN(cardcontext->ctx, r);
	}

	if ((*tname)[r-1] != '\0') {
		(*tname)[r-1] = '\0';
	}