static int msgpack_object_to_ra_value(msgpack_object o,
                                      struct flb_ra_value *result)
{
    result->o = o;

    /* Compose result with found value */
    if (o.type == MSGPACK_OBJECT_BOOLEAN) {
        result->type = FLB_RA_BOOL;
        result->val.boolean = o.via.boolean;
        return 0;
    }
    else if (o.type == MSGPACK_OBJECT_POSITIVE_INTEGER ||
             o.type == MSGPACK_OBJECT_NEGATIVE_INTEGER) {
        result->type = FLB_RA_INT;
        result->val.i64 = o.via.i64;
        return 0;
    }
    else if (o.type == MSGPACK_OBJECT_FLOAT32 ||
             o.type == MSGPACK_OBJECT_FLOAT) {
        result->type = FLB_RA_FLOAT;
        result->val.f64 = o.via.f64;
        return 0;
    }
    else if // <MASK>

    return -1;
}