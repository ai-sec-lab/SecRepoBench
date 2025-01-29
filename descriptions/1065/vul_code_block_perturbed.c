assert(regex->rc == 0);
	return regexec(&regex->rx, str, nmatch, pmatch, eflags);