PLIST_API int plist_from_json(const char *json, uint32_t length, plist_t * plist)
{
    if (!plist) {
        return PLIST_ERR_INVALID_ARG;
    }
    *plist = NULL;
    if (!json || (length == 0)) {
        return PLIST_ERR_INVALID_ARG;
    }

    jsmn_parser parser;
    jsmn_init(&parser);
    int maxtoks = 256;
    int curtoks = 0;
    int r = 0;
    jsmntok_t *tokens = NULL;

    do {
        jsmntok_t* newtokens = realloc(tokens, sizeof(jsmntok_t)*maxtoks);
        if (!newtokens) {
            PLIST_JSON_ERR("%s: Out of memory\n", __func__);
            return PLIST_ERR_NO_MEM;
        }
        memset((unsigned char*)newtokens + sizeof(jsmntok_t)*curtoks, '\0', sizeof(jsmntok_t)*(maxtoks-curtoks));
        tokens = newtokens;
        curtoks = maxtoks;

        r = jsmn_parse(&parser, json, length, tokens, maxtoks);
        if (r == JSMN_ERROR_NOMEM) {
            maxtoks+=16;
            continue;
        }
    } while (r == JSMN_ERROR_NOMEM);

    switch(r) {
        case JSMN_ERROR_NOMEM:
            PLIST_JSON_ERR("%s: Out of memory...\n", __func__);
            free(tokens);
            return PLIST_ERR_NO_MEM;
        case JSMN_ERROR_INVAL:
            PLIST_JSON_ERR("%s: Invalid character inside JSON string\n", __func__);
            free(tokens);
            return PLIST_ERR_PARSE;
        case JSMN_ERROR_PART:
            PLIST_JSON_ERR("%s: Incomplete JSON, more bytes expected\n", __func__);
            free(tokens);
            return PLIST_ERR_PARSE;
        default:
            break;
    }

    int startindex = 0;
    // <MASK>
    free(tokens);
    return PLIST_ERR_SUCCESS;
}