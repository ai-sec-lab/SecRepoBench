if (mp_buf != tmp_out_buf) {
        flb_free(mp_buf);
        mp_buf = NULL;
    }

    /* Do time resolution ? */
    if (!parser->time_fmt) {
        msgpack_unpacked_destroy(&result);
        return *out_size;
    }

    if (parser->time_key) {
        time_key = parser->time_key;
    }
    else {
        time_key = "time";
    }
    slen = strlen(time_key);

    /* Lookup time field */
    map_size = map.via.map.size;
    skip = map_size;
    for (i = 0; i < map_size; i++) {
        k = &map.via.map.ptr[i].key;
        time_value = &map.via.map.ptr[i].val;

        if (k->via.str.size != slen) {
            continue;
        }

        /* Ensure the pointer we are about to read is not NULL */
        if (k->via.str.ptr == NULL) {
            flb_free(mp_buf);
            flb_free(tmp_out_buf);
            *out_buf = NULL;
            msgpack_unpacked_destroy(&result);
            return -1;
        }

        if (strncmp(k->via.str.ptr, time_key, k->via.str.size) == 0) {
            /* We found the key, break the loop and keep the index */
            if (parser->time_keep == FLB_FALSE) {
                skip = i;
                break;
            }
            else {
                skip = -1;
            }
            break;
        }

        k = NULL;
        time_value = NULL;
    }