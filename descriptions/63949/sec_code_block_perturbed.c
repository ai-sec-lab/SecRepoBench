int ii;
	size_t offs;
	unsigned char mask = 0x40;

	if (flags)
		acls = &docparser->acls_contactless;

	if (!acls->size)
		LOG_FUNC_RETURN(ctx, SC_ERROR_INVALID_DATA);

	docparser->amb = *(acls->value + 0);
	memset(docparser->scbs, 0xFF, sizeof(docparser->scbs));
	for (ii=0, offs = 1; ii<7; ii++, mask >>= 1)
		if (mask & docparser->amb) {
			if (offs >= acls->size) {
				LOG_FUNC_RETURN(ctx, SC_ERROR_INVALID_DATA);
			}
			docparser->scbs[ii] = *(acls->value + offs++);
		}