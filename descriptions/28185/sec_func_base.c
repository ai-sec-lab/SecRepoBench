static int idprime_get_token_name(sc_card_t* card, char** tname)
{
	idprime_private_data_t * priv = card->drv_data;
	sc_path_t tinfo_path = {"\x00\x00", 2, 0, 0, SC_PATH_TYPE_PATH, {"", 0}};
	sc_file_t *file = NULL;
	u8 buf[2];
	char *name;
	int r;

	LOG_FUNC_CALLED(card->ctx);

	if (tname == NULL) {
		LOG_FUNC_RETURN(card->ctx, SC_ERROR_INVALID_ARGUMENTS);
	}

	if (!priv->tinfo_present) {
		LOG_FUNC_RETURN(card->ctx, SC_ERROR_NOT_SUPPORTED);
	}

	memcpy(tinfo_path.value, priv->tinfo_df, 2);
	r = iso_ops->select_file(card, &tinfo_path, &file);
	if (r != SC_SUCCESS || file->size == 0) {
		sc_file_free(file);
		LOG_FUNC_RETURN(card->ctx, SC_ERROR_NOT_SUPPORTED);
	}

	/* First two bytes lists 0x01, the second indicates length */
	r = iso_ops->read_binary(card, 0, buf, 2, 0);
	if (r < 2 || buf[1] > file->size) { /* make sure we do not overrun */
		sc_file_free(file);
		LOG_FUNC_RETURN(card->ctx, r);
	}
	sc_file_free(file);

	name = malloc(buf[1]);
	if (name == NULL) {
		LOG_FUNC_RETURN(card->ctx, SC_ERROR_OUT_OF_MEMORY);
	}

	r = iso_ops->read_binary(card, 2, (unsigned char *)name, buf[1], 0);
	if (r < 1) {
		free(name);
		LOG_FUNC_RETURN(card->ctx, r);
	}

	if (name[r-1] != '\0') {
		name[r-1] = '\0';
	}
	*tname = name;

	LOG_FUNC_RETURN(card->ctx, SC_SUCCESS);
}