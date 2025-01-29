case CDF_CLIPBOARD:
			if (inp[i].pi_type & CDF_VECTOR)
				goto unknown;
			break;
		default:
		unknown:
			DPRINTF(("Don't know how to deal with %#x\n",
			    inp[i].pi_type));
			break;