
	assert(rx->rc == 0);
	return regexec(&rx->rx, str, nmatch, pmatch, eflags);
