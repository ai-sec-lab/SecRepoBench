case EXIF_FORMAT_RATIONAL:
	    if (size < exif_format_get_size (EXIF_FORMAT_RATIONAL)) return NULL;
	    if (entry->components < 1) return NULL; /* FIXME: could handle more than 1 too */
	    vr = exif_get_rational (data, entry->order);
	    if (!vr.denominator) break;
	    snprintf (v, maxlen, "%2.4f", (double) vr.numerator /
					    vr.denominator);
	    break;
	case EXIF_FORMAT_SRATIONAL:
	    if (size < exif_format_get_size (EXIF_FORMAT_SRATIONAL)) return NULL;
	    if (entry->components < 1) return NULL; /* FIXME: could handle more than 1 too */
	    vsr = exif_get_srational (data, entry->order);
	    if (!vsr.denominator) break;
	    snprintf (v, maxlen, "%2.4f", (double) vsr.numerator /
		  vsr.denominator);
	    break;
	case EXIF_FORMAT_UNDEFINED:
	default:
	    snprintf (v, maxlen, _("%i bytes unknown data"), entry->size);
	    break;