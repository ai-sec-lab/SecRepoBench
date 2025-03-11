/* Each array must have two entries: time and record */
        parsed_object = result.data;
        if (parsed_object.type != MSGPACK_OBJECT_ARRAY) {
            continue;
        }
        if (parsed_object.via.array.size != 2) {
            continue;
        }

        /* Unpack time */
        flb_time_pop_from_msgpack(&tms, &result, &obj);

        /* Get the record/map */
        map = parsed_object.via.array.ptr[1];
        if (map.type != MSGPACK_OBJECT_MAP) {
            continue;
        }
        map_size = map.via.map.size;

        if (date_key != NULL) {
            msgpack_pack_map(&tmp_pck, map_size + 1);
        }
        else {
            msgpack_pack_map(&tmp_pck, map_size);
        }