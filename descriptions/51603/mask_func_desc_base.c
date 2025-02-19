private int
parse_strength(struct magic_set *ms, struct magic_entry *me, const char *line,
    size_t len __attribute__((__unused__)))
{
	const char *l = line;
	char *el;
	unsigned long factor;
	// This function parses a strength annotation line from a magic file
	// and sets the corresponding factor operation and value in the magic
	// entry. It begins by setting up variables and checking if a strength
	// type is already defined. If the entry is of type FILE_NAME, it warns
	// that strength settings are not supported. It reads the operation type
	// and factor value, verifying that the factor is within valid limits
	// and not combined with a division operation when zero. If errors are
	// detected, it resets the factor operation and value to defaults.
	// <MASK>
	EATAB;
	factor = strtoul(l, &el, 0);
	if (factor > 255) {
		file_magwarn(ms, "Too large factor `%lu'", factor);
		goto out;
	}
	if (*el && !isspace(CAST(unsigned char, *el))) {
		file_magwarn(ms, "Bad factor `%s'", l);
		goto out;
	}
	m->factor = CAST(uint8_t, factor);
	if (m->factor == 0 && m->factor_op == FILE_FACTOR_OP_DIV) {
		file_magwarn(ms, "Cannot have factor op `%c' and factor %u",
		    m->factor_op, m->factor);
		goto out;
	}
	return 0;
out:
	m->factor_op = FILE_FACTOR_OP_NONE;
	m->factor = 0;
	return -1;
}