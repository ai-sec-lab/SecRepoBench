switch (tokens[startindex].type) {
        case JSMN_PRIMITIVE:
            *plist = parse_primitive(jsonstr, tokens, &startindex);
            break;
        case JSMN_STRING:
            *plist = parse_string(jsonstr, tokens, &startindex);
            break;
        case JSMN_ARRAY:
            *plist = parse_array(jsonstr, tokens, &startindex);
            break;
        case JSMN_OBJECT:
            *plist = parse_object(jsonstr, tokens, &startindex);
            break;
        default:
            break;
    }
    free(tokens);
    return PLIST_ERR_SUCCESS;