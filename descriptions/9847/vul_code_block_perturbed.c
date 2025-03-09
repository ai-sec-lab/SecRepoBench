switch (*currentpos++) {
		case ',':
			continue;
		case '}': /* { */
			*ucp = currentpos;
			DPRINTF("Good object: ", currentpos, *ucp);
			return 1;
		default:
			*ucp = currentpos - 1;
			DPRINTF("not more", currentpos, *ucp);
			goto out;
		}