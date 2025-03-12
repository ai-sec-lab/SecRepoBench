/* Based on data type, convert to it string representation */
    if (v->type == FLB_RA_BOOL) {
        /* Check if is a map or a real bool */
        if (v->o.type == MSGPACK_OBJECT_MAP) {
            /* Convert msgpack map to JSON string */
            js = flb_msgpack_to_json_str(1024, &v->o);
            if (js) {
                len = strlen(js);
                tmp = flb_sds_cat(buf, js, len);
                flb_free(js);
            }
        }
        else if (v->o.type == MSGPACK_OBJECT_BOOLEAN) {
            if (v->val.boolean) {
                tmp = flb_sds_cat(buf, "true", 4);
            }
            else {
                tmp = flb_sds_cat(buf, "false", 5);
            }
        }
    }
    else if (v->type == FLB_RA_INT) {
        len = snprintf(str, sizeof(str) - 1, "%" PRId64, v->val.i64);
        tmp = flb_sds_cat(buf, str, len);
    }
    else if (v->type == FLB_RA_FLOAT) {
        len = snprintf(str, sizeof(str) - 1, "%f", v->val.f64);
        tmp = flb_sds_cat(buf, str, len);
    }
    else if (v->type == FLB_RA_STRING) {
        tmp = flb_sds_cat(buf, v->val.string, flb_sds_len(v->val.string));
    }
    else if (v->type == FLB_RA_NULL) {
        tmp = flb_sds_cat(buf, "null", 4);
    }