assert(rx->rc == 0);
	/* XXX: force initialization because glibc does not always do this */
	memset(pmatch, 0, nmatch * sizeof(*pmatch));
	return regexec(&rx->rx, str, nmatch, pmatch, eflags);