static plist_t parse_primitive(const char* js, jsmntok_info_t* ti, int* index)
{
    if (ti->tokens[*index].type != JSMN_PRIMITIVE) {
        PLIST_JSON_ERR("%s: token type != JSMN_PRIMITIVE\n", __func__);
        return NULL;
    }
    plist_t parsed_value = NULL;
    const char* str_val = js + ti->tokens[*index].start;
    const char* str_end = js + ti->tokens[*index].end;
    size_t str_len = ti->tokens[*index].end - ti->tokens[*index].start;
    if (!strncmp("false", str_val, str_len)) {
        parsed_value = plist_new_bool(0);
    } else if (!strncmp("true", str_val, str_len)) {
        parsed_value = plist_new_bool(1);
    } else if (!strncmp("null", str_val, str_len)) {
        plist_data_t data = plist_new_plist_data();
        data->type = PLIST_NULL;
        parsed_value = plist_new_node(data);
    } else if (str_val[0] == '-' || isdigit(str_val[0])) {
        char* endp = NULL;
        char cbuf[48];
        size_t maxlen = str_end-str_val;
        if (maxlen >= sizeof(cbuf)) maxlen = sizeof(cbuf)-1;
        strncpy(cbuf, str_val, maxlen);
        cbuf[maxlen] = '\0';
        long long intpart = strtoll(cbuf, &endp, 10);
        endp = (char*)str_val + (endp-&cbuf[0]);
        if (endp >= str_end) {
            /* integer */
            parsed_value = plist_new_uint((uint64_t)intpart);
        } else if (*endp == '.' && endp+1 < str_end && isdigit(*(endp+1))) {
            /* float */
            char* fendp = endp+1;
            while (isdigit(*fendp) && fendp < str_end) fendp++;
            if ((fendp > endp+1 && fendp >= str_end) || (fendp+2 < str_end && (*fendp == 'e' || *fendp == 'E') && (*(fendp+1) == '+' || *(fendp+1) == '-') && isdigit(*(fendp+2)))) {
                double dval = atof(cbuf);
                parsed_value = plist_new_real(dval);
            } else {
                PLIST_JSON_ERR("%s: invalid character at offset %d when parsing floating point value\n", __func__, (int)(fendp - js));
            }
        } else {
            PLIST_JSON_ERR("%s: invalid character at offset %d when parsing numerical value\n", __func__, (int)(endp - js));
        }
    } else {
        PLIST_JSON_ERR("%s: invalid primitive value '%.*s' encountered\n", __func__, (int)str_len, str_val);
    }
    (*index)++;
    return parsed_value;
}