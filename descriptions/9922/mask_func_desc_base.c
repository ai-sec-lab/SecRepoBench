static int
json_parse_string(const unsigned char **ucp, const unsigned char *ue)
{
	const unsigned char *uc = *ucp;
	size_t i;

	DPRINTF("Parse string: ", uc, *ucp);
	while (uc < ue) {
		switch (*uc++) {
		case '\0':
			goto out;
		case '\\':
			// Handle escape sequences inside a JSON string. Advance the pointer `uc` for
			// valid escape sequences: `\\`, `\"`, `\/`, `\b`, `\f`, `\n`, `\r`, `\t`, and
			// `\u` followed by 4 hexadecimal digits. If an invalid escape sequence or 
			// unexpected end of data is encountered, exit the parsing loop.
			// <MASK>
		case '"':
			*ucp = uc;
			return 1;
		default:
			continue;
		}
	}
out:
	DPRINTF("Bad string: ", uc, *ucp);
	*ucp = uc;
	return 0;
}