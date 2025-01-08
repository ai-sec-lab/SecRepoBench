while (ue - uc >= CAST(ptrdiff_t, sizeof(nbytes))) {
		nbytes = getlen(&uc);
		if ((nt > 0 || nr > 0) && nbytes == 0xFFFFFFFF)
			/* EOM after at least one record or tapemark */
			break;
		if (nbytes == 0) {
			nt++;	/* count tapemarks */
#if SIMH_TAPEMARKS
			if (nt == SIMH_TAPEMARKS)
				break;
#endif
			continue;
		}
		/* handle a data record */
		uc += nbytes;
		if (ue - uc < CAST(ptrdiff_t, sizeof(nbytes)))
			break;
		cbytes = getlen(&uc);
		if (nbytes != cbytes)
			return 0;
		nr++;
	}