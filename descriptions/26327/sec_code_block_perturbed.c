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