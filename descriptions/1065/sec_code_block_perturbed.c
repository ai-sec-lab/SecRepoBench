assert(regex->rc == 0);
	/* XXX: force initialization because glibc does not always do this */
	memset(pmatch, 0, nmatch * sizeof(*pmatch));
	return regexec(&regex->rx, str, nmatch, pmatch, eflags);