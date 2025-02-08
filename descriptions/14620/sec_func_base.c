static status
searchObjectForKeyRec(const char *searchKey, CtxJson *ctx, 
                      ParseCtx *parseCtx, size_t *resultIndex, UA_UInt16 depth) {
    UA_StatusCode ret = UA_STATUSCODE_BADNOTFOUND;
    
    CHECK_TOKEN_BOUNDS;
    
    if(parseCtx->tokenArray[parseCtx->index].type == JSMN_OBJECT) {
        size_t objectCount = (size_t)parseCtx->tokenArray[parseCtx->index].size;
        parseCtx->index++; /*Object to first Key*/
        
        for(size_t i = 0; i < objectCount; i++) {
            CHECK_TOKEN_BOUNDS;
            if(depth == 0) { /* we search only on first layer */
                if(jsoneq((char*)ctx->pos, &parseCtx->tokenArray[parseCtx->index], searchKey) == 0) {
                    /*found*/
                    parseCtx->index++; /*We give back a pointer to the value of the searched key!*/
                    if (parseCtx->index >= parseCtx->tokenCount)
                        /* We got invalid json. See https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=14620 */
                        return UA_STATUSCODE_BADOUTOFRANGE;
                    *resultIndex = parseCtx->index;
                    return UA_STATUSCODE_GOOD;
                }
            }
               
            parseCtx->index++; /* value */
            CHECK_TOKEN_BOUNDS;
            
            if(parseCtx->tokenArray[parseCtx->index].type == JSMN_OBJECT) {
               ret = searchObjectForKeyRec(searchKey, ctx, parseCtx, resultIndex,
                                           (UA_UInt16)(depth + 1));
            } else if(parseCtx->tokenArray[parseCtx->index].type == JSMN_ARRAY) {
               ret = searchObjectForKeyRec(searchKey, ctx, parseCtx, resultIndex,
                                           (UA_UInt16)(depth + 1));
            } else {
                /* Only Primitive or string */
                parseCtx->index++;
            }
        }
    } else if(parseCtx->tokenArray[parseCtx->index].type == JSMN_ARRAY) {
        size_t arraySize = (size_t)parseCtx->tokenArray[parseCtx->index].size;
        parseCtx->index++; /*Object to first element*/
        
        for(size_t i = 0; i < arraySize; i++) {
            CHECK_TOKEN_BOUNDS;
            if(parseCtx->tokenArray[parseCtx->index].type == JSMN_OBJECT) {
               ret = searchObjectForKeyRec(searchKey, ctx, parseCtx, resultIndex,
                                           (UA_UInt16)(depth + 1));
            } else if(parseCtx->tokenArray[parseCtx->index].type == JSMN_ARRAY) {
               ret = searchObjectForKeyRec(searchKey, ctx, parseCtx, resultIndex,
                                           (UA_UInt16)(depth + 1));
            } else {
                /* Only Primitive or string */
                parseCtx->index++;
            }
        }
    }
    return ret;
}