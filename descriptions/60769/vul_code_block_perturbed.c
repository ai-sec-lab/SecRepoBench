unsigned char *content, last_update[32];
	size_t lupdate_len = sizeof(last_update) - 1;
	int r, content_len;
	size_t size;

	if (pkcscard->tokeninfo->last_update.gtime)
		goto done;

	if (!pkcscard->tokeninfo->last_update.path.len)
		return NULL;

	r = sc_select_file(pkcscard->card, &pkcscard->tokeninfo->last_update.path, &file);
	if (r < 0)
		return NULL;

	size = file->size ? file->size : 1024;
	sc_file_free(file);

	content = calloc(size, 1);
	if (!content)
		return NULL;

	r = sc_read_binary(pkcscard->card, 0, content, size, 0);
	if (r < 0) {
		free(content);
		return NULL;
	}
	content_len = r;

	sc_copy_asn1_entry(c_asn1_last_update, asn1_last_update);
	sc_format_asn1_entry(asn1_last_update + 0, last_update, &lupdate_len, 0);

	r = sc_asn1_decode(ctx, asn1_last_update, content, content_len, NULL, NULL);
	free(content);
	if (r < 0)
		return NULL;

	pkcscard->tokeninfo->last_update.gtime = strdup((char *)last_update);
	if (!pkcscard->tokeninfo->last_update.gtime)
		return NULL;