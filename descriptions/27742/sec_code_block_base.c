if (root.type != MSGPACK_OBJECT_ARRAY) {
            continue;
        }
        if (root.via.array.size != 2) {
            continue;
        }

        /* Unpack time */
        flb_time_pop_from_msgpack(&tms, &result, &obj);

        /* Get the record/map */
        map = root.via.array.ptr[1];
        if (map.type != MSGPACK_OBJECT_MAP) {
            continue;
        }