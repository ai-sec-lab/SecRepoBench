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

    // <MASK>

    flb_ra_key_value_destroy(v);
    return tmp;
}