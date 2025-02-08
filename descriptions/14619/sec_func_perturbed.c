static status
searchObjectForKeyRec(const char *searchKey, CtxJson *ctx, 
                      ParseCtx *parserContext, size_t *resultIndex, UA_UInt16 depth) {
    UA_StatusCode ret = UA_STATUSCODE_BADNOTFOUND;
    
    CHECK_TOKEN_BOUNDS;
    
    if(parserContext->tokenArray[parserContext->index].type == JSMN_OBJECT) {
        size_t objectCount = (size_t)(parserContext->tokenArray[parserContext->index].size);
        parserContext->index++; /*Object to first Key*/
        
        for(size_t i = 0; i < objectCount; i++) {
            CHECK_TOKEN_BOUNDS;
            if(depth == 0) { /* we search only on first layer */
                if(jsoneq((char*)ctx->pos, &parserContext->tokenArray[parserContext->index], searchKey) == 0) {
                    /*found*/
                    parserContext->index++; /*We give back a pointer to the value of the searched key!*/
                    *resultIndex = parserContext->index;
                    ret = UA_STATUSCODE_GOOD;
                    break;
                }
            }
               
            parserContext->index++; /* value */
            CHECK_TOKEN_BOUNDS;
            
            if(parserContext->tokenArray[parserContext->index].type == JSMN_OBJECT) {
               ret = searchObjectForKeyRec(searchKey, ctx, parserContext, resultIndex,
                                           (UA_UInt16)(depth + 1));
            } else if(parserContext->tokenArray[parserContext->index].type == JSMN_ARRAY) {
               ret = searchObjectForKeyRec(searchKey, ctx, parserContext, resultIndex,
                                           (UA_UInt16)(depth + 1));
            } else {
                /* Only Primitive or string */
                parserContext->index++;
            }
        }
    } else if(parserContext->tokenArray[parserContext->index].type == JSMN_ARRAY) {
        size_t arraySize = (size_t)(parserContext->tokenArray[parserContext->index].size);
        parserContext->index++; /*Object to first element*/
        
        for(size_t i = 0; i < arraySize; i++) {
            CHECK_TOKEN_BOUNDS;
            if(parserContext->tokenArray[parserContext->index].type == JSMN_OBJECT) {
               ret = searchObjectForKeyRec(searchKey, ctx, parserContext, resultIndex,
                                           (UA_UInt16)(depth + 1));
            } else if(parserContext->tokenArray[parserContext->index].type == JSMN_ARRAY) {
               ret = searchObjectForKeyRec(searchKey, ctx, parserContext, resultIndex,
                                           (UA_UInt16)(depth + 1));
            } else {
                /* Only Primitive or string */
                parserContext->index++;
            }
        }
    }
    return ret;
}