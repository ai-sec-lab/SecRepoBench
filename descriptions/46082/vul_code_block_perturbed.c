(object.type == MSGPACK_OBJECT_STR) {
        result->type = FLB_RA_STRING;
        result->val.string = flb_sds_create_len((char *) object.via.str.ptr,
                                                object.via.str.size);
        return 0;
    }
    else if (object.type == MSGPACK_OBJECT_MAP) {
        /* return boolean 'true', just denoting the existence of the key */
        result->type = FLB_RA_BOOL;
        result->val.boolean = true;
        return 0;
    }
    else if (object.type == MSGPACK_OBJECT_NIL) {
        result->type = FLB_RA_NULL;
        return 0;
    }