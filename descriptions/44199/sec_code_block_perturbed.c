jsmntok_info_t ti = { tokens, parser.toknext };
    switch (tokens[startindex].type) {
        case JSMN_PRIMITIVE:
            *plist = parse_primitive(jsonstr, &ti, &startindex);
            break;
        case JSMN_STRING:
            *plist = parse_string(jsonstr, &ti, &startindex);
            break;
        case JSMN_ARRAY:
            *plist = parse_array(jsonstr, &ti, &startindex);
            break;
        case JSMN_OBJECT:
            *plist = parse_object(jsonstr, &ti, &startindex);
            break;
        default:
            break;
    }