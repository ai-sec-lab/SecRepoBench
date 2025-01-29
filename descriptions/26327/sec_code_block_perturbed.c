if (time_len >= sizeof(tmp)) {
            return -1;
        }
        memcpy(tmp, time_ptr, time_len);
        tmp[time_len] = '\0';
        time_ptr = tmp;
        time_len = strlen(tmp);

        p = flb_strptime(time_ptr, timeparser->time_fmt, tm);