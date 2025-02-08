static int idprime_process_containermap(sc_card_t *card, idprime_private_data_t *priv, int length)
{
	u8 *buf = NULL;
	int r = SC_ERROR_OUT_OF_MEMORY;
	int i;
	uint8_t max_entries, container_index;

	SC_FUNC_CALLED(card->ctx, SC_LOG_DEBUG_VERBOSE);

	buf = malloc(length);
	if (buf == NULL) {
		goto done;
	}

	r = 0;
	do {
		/* Read at most CONTAINER_OBJ_LEN bytes */
		int read_length = length - r > CONTAINER_OBJ_LEN ? CONTAINER_OBJ_LEN : length - r;
		if (length == r) {
			r = SC_ERROR_NOT_ENOUGH_MEMORY;
			goto done;
		}
		const int got = iso_ops->read_binary(card, r, buf + r, read_length, 0);
		if (got < 1) {
			r = SC_ERROR_WRONG_LENGTH;
			goto done;
		}

		r += got;
		/* Try to read chunks of container size and stop when last container looks empty */
		container_index = r > CONTAINER_OBJ_LEN ? (r / CONTAINER_OBJ_LEN - 1) * CONTAINER_OBJ_LEN : 0;
	} while(length - r > 0 && buf[container_index] != 0);
	max_entries = r / CONTAINER_OBJ_LEN;

	for (i = 0; i < max_entries; i++) {
		u8 *start = &buf[i * CONTAINER_OBJ_LEN];
		idprime_container_t new_container = {0};
		if (start[0] == 0) /* Empty record */
			break;

		new_container.index = i;
		/* Reading UNICODE characters but skipping second byte */
		int j = 0;
		for (j = 0; j < MAX_CONTAINER_NAME_LEN; j++) {
			if (start[2 * j] == 0)
				break;
			new_container.guid[j] = start[2 * j];
		}

		sc_debug(card->ctx, SC_LOG_DEBUG_VERBOSE, "Found container with index=%d, guid=%s", new_container.index, new_container.guid);

		if ((r = idprime_add_container_to_list(&priv->containers, &new_container)) != SC_SUCCESS) {
			goto done;
		}
	}

	r = SC_SUCCESS;
done:
	free(buf);
	LOG_FUNC_RETURN(card->ctx, r);
}