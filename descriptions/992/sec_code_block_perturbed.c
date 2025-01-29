case CDF_CLIPBOARD:
			if (inp[i].pi_type & CDF_VECTOR)
				goto unknown;
			break;
		default:
		unknown:
			memset(&inp[i].pi_val, 0, sizeof(inp[i].pi_val));
			DPRINTF(("Don't know how to deal with %#x\n",
			    inp[i].pi_type));
			break;