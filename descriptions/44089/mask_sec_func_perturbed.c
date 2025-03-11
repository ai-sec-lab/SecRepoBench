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
        // <MASK>
    } else {
        PLIST_JSON_ERR("%s: invalid primitive value '%.*s' encountered\n", __func__, (int)str_len, str_val);
    }
    (*index)++;
    return parsed_value;
}