static char *
_flb_strptime(const char *buf, const char *fmt, struct tm *tm, int initialize)
{
	unsigned char c;
	const unsigned char *bp, *ep;
	size_t len = 0;
	int alt_format, i, offs;
	int neg = 0;
	static int century, relyear, fields;

	if (initialize) {
		century = TM_YEAR_BASE;
		relyear = -1;
		fields = 0;
	}

	bp = (const unsigned char *)buf;
	while ((c = *fmt) != '\0') {
		/* Clear `alternate' modifier prior to new conversion. */
		alt_format = 0;

		/* Eat up white-space. */
		if (isspace(c)) {
			while (isspace(*bp))
				bp++;

			fmt++;
			continue;
		}

        /*
         * Having increased bp we need to ensure we are not
         * moving beyond bounds.
         */
        // <MASK>


	}

	/*
	 * We need to evaluate the two digit year spec (%y)
	 * last as we can get a century spec (%C) at any time.
	 */
	if (relyear != -1) {
		if (century == TM_YEAR_BASE) {
			if (relyear <= 68)
				tm->tm_year = relyear + 2000 - TM_YEAR_BASE;
			else
				tm->tm_year = relyear + 1900 - TM_YEAR_BASE;
		} else {
			tm->tm_year = relyear + century - TM_YEAR_BASE;
		}
		fields |= FIELD_TM_YEAR;
	}

	/* Compute some missing values when possible. */
	if (fields & FIELD_TM_YEAR) {
		const int year = tm->tm_year + TM_YEAR_BASE;
		const int *mon_lens = mon_lengths[isleap(year)];
		if (!(fields & FIELD_TM_YDAY) &&
		    (fields & FIELD_TM_MON) && (fields & FIELD_TM_MDAY)) {
			tm->tm_yday = tm->tm_mday - 1;
			for (i = 0; i < tm->tm_mon; i++)
				tm->tm_yday += mon_lens[i];
			fields |= FIELD_TM_YDAY;
		}
		if (fields & FIELD_TM_YDAY) {
			int days = tm->tm_yday;
			if (!(fields & FIELD_TM_WDAY)) {
				tm->tm_wday = EPOCH_WDAY +
				    ((year - EPOCH_YEAR) % DAYSPERWEEK) *
				    (DAYSPERNYEAR % DAYSPERWEEK) +
				    leaps_thru_end_of(year - 1) -
				    leaps_thru_end_of(EPOCH_YEAR - 1) +
				    tm->tm_yday;
				tm->tm_wday %= DAYSPERWEEK;
				if (tm->tm_wday < 0)
					tm->tm_wday += DAYSPERWEEK;
			}
			if (!(fields & FIELD_TM_MON)) {
				tm->tm_mon = 0;
				while (tm->tm_mon < MONSPERYEAR && days >= mon_lens[tm->tm_mon])
					days -= mon_lens[tm->tm_mon++];
			}
			if (!(fields & FIELD_TM_MDAY))
				tm->tm_mday = days + 1;
		}
	}

	return ((char *)bp);
}