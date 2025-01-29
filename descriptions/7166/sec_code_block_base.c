#ifdef TVB_Z_DEBUG
	guint      inflate_passes = 0;
	guint      bytes_in       = tvb_captured_length_remaining(tvb, offset);
#endif

	if (tvb == NULL || comprlen <= 0) {
		return NULL;
	}

	compr = (guint8 *)tvb_memdup(NULL, tvb, offset, comprlen);