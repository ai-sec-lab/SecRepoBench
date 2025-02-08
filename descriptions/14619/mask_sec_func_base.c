static status
searchObjectForKeyRec(const char *searchKey, CtxJson *ctx, 
                      ParseCtx *parseCtx, size_t *resultIndex, UA_UInt16 depth) {
    UA_StatusCode ret = UA_STATUSCODE_BADNOTFOUND;
    
    CHECK_TOKEN_BOUNDS;
    
    if(parseCtx->tokenArray[parseCtx->index].type == JSMN_OBJECT) // <MASK>
    return ret;
}