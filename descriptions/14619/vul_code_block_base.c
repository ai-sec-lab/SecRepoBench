{
        size_t objectCount = (size_t)(parseCtx->tokenArray[parseCtx->index].size);
        
        parseCtx->index++; /*Object to first Key*/
        CHECK_TOKEN_BOUNDS;
        
        size_t i;
        for(i = 0; i < objectCount; i++) {
            
            CHECK_TOKEN_BOUNDS;
            if(depth == 0) { /* we search only on first layer */
                if(jsoneq((char*)ctx->pos, &parseCtx->tokenArray[parseCtx->index], searchKey) == 0) {
                    /*found*/
                    parseCtx->index++; /*We give back a pointer to the value of the searched key!*/
                    *resultIndex = parseCtx->index;
                    ret = UA_STATUSCODE_GOOD;
                    break;
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
        size_t arraySize = (size_t)(parseCtx->tokenArray[parseCtx->index].size);
        
        parseCtx->index++; /*Object to first element*/
        CHECK_TOKEN_BOUNDS;
        
        size_t i;
        for(i = 0; i < arraySize; i++) {
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