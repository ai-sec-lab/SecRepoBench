jsmntok_info_t ti = { tokens, parser.toknext };
    switch (tokens[startindex].type) {
        case JSMN_PRIMITIVE:
            *plist = parse_primitive(json, &ti, &startindex);
            break;
        case JSMN_STRING:
            *plist = parse_string(json, &ti, &startindex);
            break;
        case JSMN_ARRAY:
            *plist = parse_array(json, &ti, &startindex);
            break;
        case JSMN_OBJECT:
            *plist = parse_object(json, &ti, &startindex);
            break;
        default:
            break;
    }
    free(tokens);
    return PLIST_ERR_SUCCESS;