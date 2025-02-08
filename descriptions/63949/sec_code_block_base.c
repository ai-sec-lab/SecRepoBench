int ii;
	size_t offs;
	unsigned char mask = 0x40;

	if (flags)
		acls = &docp->acls_contactless;

	if (!acls->size)
		LOG_FUNC_RETURN(ctx, SC_ERROR_INVALID_DATA);

	docp->amb = *(acls->value + 0);
	memset(docp->scbs, 0xFF, sizeof(docp->scbs));
	for (ii=0, offs = 1; ii<7; ii++, mask >>= 1)
		if (mask & docp->amb) {
			if (offs >= acls->size) {
				LOG_FUNC_RETURN(ctx, SC_ERROR_INVALID_DATA);
			}
			docp->scbs[ii] = *(acls->value + offs++);
		}