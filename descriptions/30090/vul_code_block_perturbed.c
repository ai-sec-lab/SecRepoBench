if (index >= map_size || !k || !v) {
        msgpack_unpacked_destroy(&result);
        return *out_size;
    }

    /* Lookup time */
    ret = flb_parser_time_lookup(v->via.str.ptr, v->via.str.size,
                                 0, parser, &tm, &tmfrac);
    if (ret == -1) {
        len = v->via.str.size;
        if (len > sizeof(tmp) - 1) {
            len = sizeof(tmp) - 1;
        }
        memcpy(tmp, v->via.str.ptr, len);
        tmp[len] = '\0';
        flb_warn("[parser:%s] invalid time format %s for '%s'",
                 parser->name, parser->time_fmt_full, tmp);
        time_lookup = 0;
    }
    else {
        time_lookup = flb_parser_tm2time(&tm);
    }