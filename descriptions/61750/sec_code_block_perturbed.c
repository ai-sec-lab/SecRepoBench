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