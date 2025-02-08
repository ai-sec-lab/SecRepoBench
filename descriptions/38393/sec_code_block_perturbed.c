size_t ofs;

	if (p >= e) {
		DPRINTF(("Past end %p < %p\n", e, p));
		return NULL;
	}

	if (cdf_check_stream_offset(streamset, h, p, (tail + 1) * sizeof(uint32_t),
	    __LINE__) == -1)
		return NULL;

	ofs = CDF_GETUINT32(p, tail);
	if (ofs < 2 * sizeof(uint32_t)) {
		DPRINTF(("Offset too small %zu\n", ofs));
		return NULL;
	}

	ofs -= 2 * sizeof(uint32_t);
	if (ofs > CAST(size_t, e - p)) {
		DPRINTF(("Offset too big %zu %td\n", ofs, e - p));
		return NULL;
	}

	return CAST(const uint8_t *, cdf_offset(CAST(const void *, p), ofs));