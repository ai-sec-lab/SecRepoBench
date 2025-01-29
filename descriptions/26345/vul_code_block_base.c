if (strstr(p->time_fmt, "%Y") || strstr(p->time_fmt, "%y")) {
            p->time_with_year = FLB_TRUE;
        }
        else if (strstr(p->time_fmt, "%s")) {
            is_epoch = FLB_TRUE;
            p->time_with_year = FLB_TRUE;
        }
        else {
            size = strlen(p->time_fmt);
            p->time_with_year = FLB_FALSE;
            p->time_fmt_year = flb_malloc(size + 4);
            if (!p->time_fmt_year) {
                flb_errno();
                flb_parser_destroy(p);
                return NULL;
            }

            /* Append the year at the beginning */
            tmp = p->time_fmt_year;
            *tmp++ = '%';
            *tmp++ = 'Y';
            *tmp++ = ' ';

            memcpy(tmp, p->time_fmt, size);
            tmp += size;
            *tmp++ = '\0';
        }

        /* Check if the format contains a timezone (%z) */
        if (strstr(p->time_fmt, "%z") || strstr(p->time_fmt, "%Z") ||
            strstr(p->time_fmt, "%SZ") || strstr(p->time_fmt, "%S.%LZ")) {
#if defined(FLB_HAVE_GMTOFF) || !defined(FLB_HAVE_SYSTEM_STRPTIME)
            p->time_with_tz = FLB_TRUE;
#else
            flb_error("[parser] timezone offset not supported");
            flb_error("[parser] you cannot use %%z/%%Z on this platform");
            flb_parser_destroy(p);
            return NULL;
#endif
        }

        /*
         * Check if the format expect fractional seconds
         *
         * Since strptime(3) does not support fractional seconds, this
         * requires a workaround/hack in our parser. This is a known
         * issue and addressed in different ways in other languages.
         *
         * The following links are a good reference:
         *
         * - http://stackoverflow.com/questions/7114690/how-to-parse-syslog-timestamp
         * - http://code.activestate.com/lists/python-list/521885/
         */
        if (is_epoch == FLB_TRUE || p->time_with_year == FLB_TRUE) {
            timeptr = p->time_fmt;
        }
        else {
            timeptr = p->time_fmt_year;
        }

        tmp = strstr(timeptr, "%L");
        if (tmp) {
            tmp[0] = '\0';
            tmp[1] = '\0';
            p->time_frac_secs = (tmp + 2);
        }

        /* Optional fixed timezone offset */
        if (time_offset) {
            diff = 0;
            len = strlen(time_offset);
            ret = flb_parser_tzone_offset(time_offset, len, &diff);
            if (ret == -1) {
                flb_parser_destroy(p);
                return NULL;
            }
            p->time_offset = diff;
        }