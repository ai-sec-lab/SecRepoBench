static int msgpack_object_to_ra_value(msgpack_object object,
                                      struct flb_ra_value *result)
{
    result->o = object;

    /* Compose result with found value */
    if (object.type == MSGPACK_OBJECT_BOOLEAN) {
        result->type = FLB_RA_BOOL;
        result->val.boolean = object.via.boolean;
        return 0;
    }
    else if (object.type == MSGPACK_OBJECT_POSITIVE_INTEGER ||
             object.type == MSGPACK_OBJECT_NEGATIVE_INTEGER) {
        result->type = FLB_RA_INT;
        result->val.i64 = object.via.i64;
        return 0;
    }
    else if (object.type == MSGPACK_OBJECT_FLOAT32 ||
             object.type == MSGPACK_OBJECT_FLOAT) {
        result->type = FLB_RA_FLOAT;
        result->val.f64 = object.via.f64;
        return 0;
    }
    else if // <MASK>

    return -1;
}