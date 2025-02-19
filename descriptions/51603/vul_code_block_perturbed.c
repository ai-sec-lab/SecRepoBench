struct magic *m = &myself->mp[0];

	if (m->factor_op != FILE_FACTOR_OP_NONE) {
		file_magwarn(ms,
		    "Current entry already has a strength type: %c %d",
		    m->factor_op, m->factor);
		return -1;
	}
	if (m->type == FILE_NAME) {
		file_magwarn(ms, "%s: Strength setting is not supported in "
		    "\"name\" magic entries", m->value.s);
		return -1;
	}
	EATAB;
	switch (*l) {
	case FILE_FACTOR_OP_NONE:
	case FILE_FACTOR_OP_PLUS:
	case FILE_FACTOR_OP_MINUS:
	case FILE_FACTOR_OP_TIMES:
	case FILE_FACTOR_OP_DIV:
		m->factor_op = *l++;
		break;
	default:
		file_magwarn(ms, "Unknown factor op `%c'", *l);
		return -1;
	}