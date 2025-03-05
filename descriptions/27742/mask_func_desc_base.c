flb_sds_t flb_pack_msgpack_to_json_format(const char *data, uint64_t bytes,
                                          int json_format, int date_format,
                                          flb_sds_t date_key)
{
    int i;
    int len;
    int ok = MSGPACK_UNPACK_SUCCESS;
    int records = 0;
    int map_size;
    size_t off = 0;
    char time_formatted[32];
    size_t s;
    flb_sds_t out_tmp;
    flb_sds_t out_js;
    flb_sds_t out_buf = NULL;
    msgpack_unpacked result;
    msgpack_object root;
    msgpack_object map;
    msgpack_sbuffer tmp_sbuf;
    msgpack_packer tmp_pck;
    msgpack_object *obj;
    msgpack_object *k;
    msgpack_object *v;
    struct tm tm;
    struct flb_time tms;

    /* Iterate the original buffer and perform adjustments */
    records = flb_mp_count(data, bytes);
    if (records <= 0) {
        return NULL;
    }

    /* For json lines and streams mode we need a pre-allocated buffer */
    if (json_format == FLB_PACK_JSON_FORMAT_LINES ||
        json_format == FLB_PACK_JSON_FORMAT_STREAM) {
        out_buf = flb_sds_create_size(bytes + bytes / 4);
        if (!out_buf) {
            flb_errno();
            return NULL;
        }
    }

    /* Create temporary msgpack buffer */
    msgpack_sbuffer_init(&tmp_sbuf);
    msgpack_packer_init(&tmp_pck, &tmp_sbuf, msgpack_sbuffer_write);

    /*
     * If the format is the original msgpack style of one big array,
     * registrate the array, otherwise is not necessary. FYI, original format:
     *
     * [
     *   [timestamp, map],
     *   [timestamp, map],
     *   [T, M]...
     * ]
     */
    if (json_format == FLB_PACK_JSON_FORMAT_JSON) {
        msgpack_pack_array(&tmp_pck, records);
    }

    msgpack_unpacked_init(&result);
    while (msgpack_unpack_next(&result, data, bytes, &off) == ok) {
        // Extract the root object from the unpacked result and ensure it is an array
        // with exactly two elements: a timestamp and a record. If these conditions
        // are not met, skip the current iteration. Unpack the timestamp using a helper
        // function and assign the second element of the array to a map object. Determine
        // the size of the map. If a date key is provided, increment the map size by one
        // to accommodate the additional date field when packing into the temporary
        // message pack buffer; otherwise, use the original map size for packing.
        // <MASK>

        if (date_key != NULL) {
            /* Append date key */
            msgpack_pack_str(&tmp_pck, flb_sds_len(date_key));
            msgpack_pack_str_body(&tmp_pck, date_key, flb_sds_len(date_key));

            /* Append date value */
            switch (date_format) {
            case FLB_PACK_JSON_DATE_DOUBLE:
                msgpack_pack_double(&tmp_pck, flb_time_to_double(&tms));
                break;
            case FLB_PACK_JSON_DATE_ISO8601:
            /* Format the time, use microsecond precision not nanoseconds */
                gmtime_r(&tms.tm.tv_sec, &tm);
                s = strftime(time_formatted, sizeof(time_formatted) - 1,
                             FLB_PACK_JSON_DATE_ISO8601_FMT, &tm);

                len = snprintf(time_formatted + s,
                               sizeof(time_formatted) - 1 - s,
                               ".%06" PRIu64 "Z",
                               (uint64_t) tms.tm.tv_nsec / 1000);
                s += len;
                msgpack_pack_str(&tmp_pck, s);
                msgpack_pack_str_body(&tmp_pck, time_formatted, s);
                break;
            case FLB_PACK_JSON_DATE_EPOCH:
                msgpack_pack_uint64(&tmp_pck, (long long unsigned)(tms.tm.tv_sec));
                break;
            }
        }

        /* Append remaining keys/values */
        for (i = 0; i < map_size; i++) {
            k = &map.via.map.ptr[i].key;
            v = &map.via.map.ptr[i].val;
            msgpack_pack_object(&tmp_pck, *k);
            msgpack_pack_object(&tmp_pck, *v);
        }

        /*
         * If the format is the original msgpack style, just continue since
         * we don't care about separator or JSON convertion at this point.
         */
        if (json_format == FLB_PACK_JSON_FORMAT_JSON) {
            continue;
        }

        /*
         * Here we handle two types of records concatenation:
         *
         * FLB_PACK_JSON_FORMAT_LINES: add  breakline (\n) after each record
         *
         *
         *     {'ts':abc,'k1':1}
         *     {'ts':abc,'k1':2}
         *     {N}
         *
         * FLB_PACK_JSON_FORMAT_STREAM: no separators, e.g:
         *
         *     {'ts':abc,'k1':1}{'ts':abc,'k1':2}{N}
         */
        if (json_format == FLB_PACK_JSON_FORMAT_LINES ||
            json_format == FLB_PACK_JSON_FORMAT_STREAM) {

            /* Encode current record into JSON in a temporary variable */
            out_js = flb_msgpack_raw_to_json_sds(tmp_sbuf.data, tmp_sbuf.size);
            if (!out_js) {
                msgpack_sbuffer_destroy(&tmp_sbuf);
                flb_sds_destroy(out_buf);
                return NULL;
            }

            /*
             * One map record has been converted, now append it to the
             * outgoing out_buf sds variable.
             */
            out_tmp = flb_sds_cat(out_buf, out_js, flb_sds_len(out_js));
            if (!out_tmp) {
                msgpack_sbuffer_destroy(&tmp_sbuf);
                flb_sds_destroy(out_js);
                flb_sds_destroy(out_buf);
                return NULL;
            }

            /* Release temporary json sds buffer */
            flb_sds_destroy(out_js);

            /* If a realloc happened, check the returned address */
            if (out_tmp != out_buf) {
                out_buf = out_tmp;
            }

            /* Append the breakline only for json lines mode */
            if (json_format == FLB_PACK_JSON_FORMAT_LINES) {
                out_tmp = flb_sds_cat(out_buf, "\n", 1);
                if (!out_tmp) {
                    msgpack_sbuffer_destroy(&tmp_sbuf);
                    flb_sds_destroy(out_buf);
                    return NULL;
                }
                if (out_tmp != out_buf) {
                    out_buf = out_tmp;
                }
            }
            msgpack_sbuffer_clear(&tmp_sbuf);
        }
    }

    /* Release the unpacker */
    msgpack_unpacked_destroy(&result);

    /* Format to JSON */
    if (json_format == FLB_PACK_JSON_FORMAT_JSON) {
        out_buf = flb_msgpack_raw_to_json_sds(tmp_sbuf.data, tmp_sbuf.size);
        msgpack_sbuffer_destroy(&tmp_sbuf);

        if (!out_buf) {
            return NULL;
        }
    }
    else {
        msgpack_sbuffer_destroy(&tmp_sbuf);
    }

    if (out_buf && flb_sds_len(out_buf) == 0) {
        flb_sds_destroy(out_buf);
        return NULL;
    }

    return out_buf;
}