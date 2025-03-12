static flb_sds_t ra_translate_keymap(struct flb_ra_parser *rp, flb_sds_t buf,
                                     msgpack_object map, int *found)
{
    int str_size;
    char *js;
    char str[32];
    flb_sds_t tmp = NULL;
    struct flb_ra_value *v;

    /* Lookup key or subkey value */
    v = flb_ra_key_to_value(rp->key->name, map, rp->key->subkeys);
    if (!v) {
        *found = FLB_FALSE;
        return buf;
    }
    else {
        *found = FLB_TRUE;
    }

    /* Based on data type, convert to it string representation */
    if (v->type == FLB_RA_BOOL) {
        /* Check if is a map or a real bool */
        if (v->o.type == MSGPACK_OBJECT_MAP) {
            /* Convert msgpack map to JSON string */
            js = flb_msgpack_to_json_str(1024, &v->o);
            if (js) {
                str_size = strlen(js);
                tmp = flb_sds_cat(buf, js, str_size);
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
        str_size = snprintf(str, sizeof(str) - 1, "%" PRId64, v->val.i64);
        /* We need to check size is not above str length */
        if (str_size >= 32) {
            *found = FLB_FALSE;
            return buf;
        }
        tmp = flb_sds_cat(buf, str, str_size);
    }
    else if (v->type == FLB_RA_FLOAT) {
        str_size = snprintf(str, sizeof(str) - 1, "%f", v->val.f64);
        /* We need to check size is not above str length */
        if (str_size >= 32) {
            *found = FLB_FALSE;
            return buf;
        }
        tmp = flb_sds_cat(buf, str, str_size);
    }
    else if (v->type == FLB_RA_STRING) {
        tmp = flb_sds_cat(buf, v->val.string, flb_sds_len(v->val.string));
    }
    else if (v->type == FLB_RA_NULL) {
        tmp = flb_sds_cat(buf, "null", 4);
    }

    flb_ra_key_value_destroy(v);
    return tmp;
}