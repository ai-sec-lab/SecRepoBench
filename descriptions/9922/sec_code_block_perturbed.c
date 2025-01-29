if (uc == endptr)
				goto out;
			switch (*uc++) {
			case '\0':
				goto out;
			case '"':
			case '\\':
			case '/':
			case 'b':
			case 'f':
			case 'n':
			case 'r':
			case 't':
				continue;
			case 'u':
				if (endptr - uc < 4) {
					uc = endptr;
					goto out;
				}
				for (i = 0; i < 4; i++)
					if (!json_isxdigit(*uc++))
						goto out;
				continue;
			default:
				goto out;
			}