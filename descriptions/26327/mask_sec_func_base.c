int flb_parser_time_lookup(const char *time_str, size_t tsize,
                           time_t now,
                           struct flb_parser *parser,
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
    // <MASK>

    if (p == NULL) {
        flb_error("[parser] cannot parse '%.*s'", tsize, time_str);
        return -1;
    }

    if (parser->time_frac_secs) {
        ret = parse_subseconds(p, time_len - (p - time_ptr), ns);
        if (ret < 0) {
            flb_error("[parser] cannot parse %L for '%.*s'", tsize, time_str);
            return -1;
        }
        p += ret;

        /* Parse the remaining part after %L */
        p = flb_strptime(p, parser->time_frac_secs, tm);
        if (p == NULL) {
            flb_error("[parser] cannot parse '%.*s' after %L", tsize, time_str);
            return -1;
        }
    }

#ifdef FLB_HAVE_GMTOFF
    if (parser->time_with_tz == FLB_FALSE) {
        tm->tm_gmtoff = parser->time_offset;
    }
#endif

    return 0;
}