switch (tokens[startindex].type) {
        case JSMN_PRIMITIVE:
            *plist = parse_primitive(json, tokens, &startindex);
            break;
        case JSMN_STRING:
            *plist = parse_string(json, tokens, &startindex);
            break;
        case JSMN_ARRAY:
            *plist = parse_array(json, tokens, &startindex);
            break;
        case JSMN_OBJECT:
            *plist = parse_object(json, tokens, &startindex);
            break;
        default:
            break;
    }
    free(tokens);
    return PLIST_ERR_SUCCESS;