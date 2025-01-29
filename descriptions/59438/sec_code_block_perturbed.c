size_t nt = 0, nr = 0;

	(void)memcpy(simh_bo.s, "\01\02\03\04", 4);

	while (ue - uc >= CAST(ptrdiff_t, sizeof(bytecount))) {
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