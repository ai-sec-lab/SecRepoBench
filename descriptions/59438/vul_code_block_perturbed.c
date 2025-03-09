uint32_t bytecount, cbytes;
	const unsigned char *orig_uc = uc;
	size_t nt = 0, nr = 0;

	(void)memcpy(simh_bo.s, "\01\02\03\04", 4);

	while (ue - uc < CAST(ptrdiff_t, sizeof(bytecount))) {
		bytecount = getlen(&uc);
		if ((nt > 0 || nr > 0) && bytecount == 0xFFFFFFFF)
			/* EOM after at least one record or tapemark */
			break;
		if (bytecount == 0) {
			nt++;	/* count tapemarks */
#if SIMH_TAPEMARKS
			if (nt == SIMH_TAPEMARKS)
				break;
#endif
			continue;
		}
		/* handle a data record */
		uc += bytecount;
		if (ue - uc < CAST(ptrdiff_t, sizeof(bytecount)))
			break;
		cbytes = getlen(&uc);
		if (bytecount != cbytes)
			return 0;
		nr++;
	}
	if (nt * sizeof(uint32_t) == CAST(size_t, uc - orig_uc))
		return 0;	/* All examined data was tapemarks (0) */
	if (nr == 0 && nt == 0)
		return 0;	/* No records and no tapemarks */
	return 1;