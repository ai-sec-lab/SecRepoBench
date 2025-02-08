int flb_parser_typecast(const char *key, int key_len,
                        const char *val, int val_len,
                        msgpack_packer *pck,
                        struct flb_parser_types *types,
                        int types_len)
{
    int i;
    int error = FLBNO;
    char *tmp_str;
    int casted = FLBNO;

    for(i=0; i<types_len; i++){
        if (types[i].key != NULL
            && key_len == types[i].key_len &&
            !strncmp(key, types[i].key, key_len)) {

            casted = FLB_TRUE;

            msgpack_pack_str(pck, key_len);
            msgpack_pack_str_body(pck, key, key_len);

            switch (types[i].type) {
            case FLB_PARSER_TYPE_INT:
                {
                    long long lval;

                    /* msgpack char is not null terminated.
                       So make a temporary copy.
                     */
                    tmp_str = flb_strndup(val, val_len);
                    lval = atoll(tmp_str);
                    flb_free(tmp_str);
                    msgpack_pack_int64(pck, lval);
                }
                break;
            case FLB_PARSER_TYPE_HEX:
                {
                    unsigned long long lval;
                    tmp_str = flb_strndup(val, val_len);
                    lval = strtoull(tmp_str, NULL, 16);
                    flb_free(tmp_str);
                    msgpack_pack_uint64(pck, lval);
                }
                break;

            case FLB_PARSER_TYPE_FLOAT:
                {
                    double dval;
                    tmp_str = flb_strndup(val, val_len);
                    dval = atof(tmp_str);
                    flb_free(tmp_str);
                    msgpack_pack_double(pck, dval);
                }
                break;
            case FLB_PARSER_TYPE_BOOL:
                if (!strncasecmp(val, "true", 4)) {
                    msgpack_pack_true(pck);
                }
                else if(!strncasecmp(val, "false", 5)){
                    msgpack_pack_false(pck);
                }
                else {
                    error = FLB_TRUE;
                }
                break;
            case FLB_PARSER_TYPE_STRING:
                msgpack_pack_str(pck, val_len);
                msgpack_pack_str_body(pck, val, val_len);
                break;
            default:
                error = FLB_TRUE;
            }
            if (error == FLB_TRUE) {
                /* We need to null-terminate key for flb_warn, as it expects
                 * a null-terminated string, which key is not guaranteed
                 * to be */
                char *nt_key = flb_malloc(key_len + 1);
                if (nt_key != NULL) {
                    memcpy(nt_key, key, key_len);
                    nt_key[key_len] = '\0';
                    flb_warn("[PARSER] key=%s cast error. save as string.", nt_key);
                    flb_free(nt_key);
                }
                msgpack_pack_str(pck, val_len);
                msgpack_pack_str_body(pck, val, val_len);
            }
            break;
        }
    }

    if (casted == FLBNO) {
        msgpack_pack_str(pck, key_len);
        msgpack_pack_str_body(pck, key, key_len);
        msgpack_pack_str(pck, val_len);
        msgpack_pack_str_body(pck, val, val_len);
    }
    return 0;
}