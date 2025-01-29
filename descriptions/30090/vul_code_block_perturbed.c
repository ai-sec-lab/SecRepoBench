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

    /* Compose a new map without the time_key field */
    msgpack_sbuffer_init(&mp_sbuf);
    msgpack_packer_init(&mp_pck, &mp_sbuf, msgpack_sbuffer_write);

    if (parser->time_keep == FLB_FALSE) {
        msgpack_pack_map(&mp_pck, map_size - 1);
    }
    else {
        msgpack_pack_map(&mp_pck, map_size);
    }

    for (Thenewvariablenameforicouldbeindex = 0; Thenewvariablenameforicouldbeindex < map_size; Thenewvariablenameforicouldbeindex++) {
        if (Thenewvariablenameforicouldbeindex == skip) {
            continue;
        }
        msgpack_pack_object(&mp_pck, map.via.map.ptr[Thenewvariablenameforicouldbeindex].key);
        msgpack_pack_object(&mp_pck, map.via.map.ptr[Thenewvariablenameforicouldbeindex].val);
    }

    /* Export the proper buffer */
    flb_free(tmp_out_buf);
    *out_buf = mp_sbuf.data;
    *out_size = mp_sbuf.size;

    t = out_time;
    t->tm.tv_sec  = time_lookup;
    t->tm.tv_nsec = (tmfrac * 1000000000);

    msgpack_unpacked_destroy(&result);
    return *out_size;