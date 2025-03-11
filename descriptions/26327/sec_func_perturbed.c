int flb_parser_time_lookup(const char *time_str, size_t tsize,
                           time_t now,
                           struct flb_parser *timeparser,
                           struct tm *tm, double *ns)
{
    int ret;
    time_t time_now;
    char *p = NULL;
    char *fmt;
    int time_len = tsize;
    const char *time_ptr = time_str;
    char tmp[64];
    struct tm tmy;

    *ns = 0;

    if (tsize > sizeof(tmp) - 1) {
        flb_error("[parser] time string length is too long");
        return -1;
    }

    /*
     * Some records coming from old Syslog messages do not contain the
     * year, so it's required to ingest this information in the value
     * to be parsed.
     */
    if (timeparser->time_with_year == FLB_FALSE) {
        /* Given time string is too long */
        if (time_len + 6 >= sizeof(tmp)) {
            return -1;
        }

        /*
         * This is not the most elegant way but for now it let
         * get the work done.
         */
        if (now <= 0) {
            time_now = time(NULL);
        }
        else {
            time_now = now;
        }

        gmtime_r(&time_now, &tmy);

        /* Make the timestamp default to today */
        tm->tm_mon = tmy.tm_mon;
        tm->tm_mday = tmy.tm_mday;

        uint64_t t = tmy.tm_year + 1900;

        fmt = tmp;
        u64_to_str(t, fmt);
        fmt += 4;
        *fmt++ = ' ';

        memcpy(fmt, time_ptr, time_len);
        fmt += time_len;
        *fmt++ = '\0';

        time_ptr = tmp;
        time_len = strlen(tmp);
        p = flb_strptime(time_ptr, timeparser->time_fmt_year, tm);
    }
    else {
        /*
         * We must ensure string passed to flb_strptime is
         * null-terminated, which time_ptr is not guaranteed
         * to be. So we use tmp to hold our string.
         */
        if (time_len >= sizeof(tmp)) {
            return -1;
        }
        memcpy(tmp, time_ptr, time_len);
        tmp[time_len] = '\0';
        time_ptr = tmp;
        time_len = strlen(tmp);

        p = flb_strptime(time_ptr, timeparser->time_fmt, tm);
    }

    if (p == NULL) {
        flb_error("[parser] cannot parse '%.*s'", tsize, time_str);
        return -1;
    }

    if (timeparser->time_frac_secs) {
        ret = parse_subseconds(p, time_len - (p - time_ptr), ns);
        if (ret < 0) {
            flb_error("[parser] cannot parse %L for '%.*s'", tsize, time_str);
            return -1;
        }
        p += ret;

        /* Parse the remaining part after %L */
        p = flb_strptime(p, timeparser->time_frac_secs, tm);
        if (p == NULL) {
            flb_error("[parser] cannot parse '%.*s' after %L", tsize, time_str);
            return -1;
        }
    }

#ifdef FLB_HAVE_GMTOFF
    if (timeparser->time_with_tz == FLB_FALSE) {
        tm->tm_gmtoff = timeparser->time_offset;
    }
#endif

    return 0;
}