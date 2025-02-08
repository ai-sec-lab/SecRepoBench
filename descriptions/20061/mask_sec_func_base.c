static status
Variant_decodeJsonUnwrapExtensionObject(UA_Variant *dst, const UA_DataType *type, 
                                        CtxJson *ctx, ParseCtx *parseCtx, UA_Boolean moveToken) {
    (void) type, (void) moveToken;
    /*EXTENSIONOBJECT POSITION!*/
    UA_UInt16 old_index = parseCtx->index;
    UA_Boolean typeIdFound;
    
    /* Decode the DataType */
    UA_NodeId typeId;
    UA_NodeId_init(&typeId);

    size_t searchTypeIdResult = 0;
    status ret = lookAheadForKey(UA_JSONKEY_TYPEID, ctx, parseCtx, &searchTypeIdResult);

    if(ret != UA_STATUSCODE_GOOD) {
        /*No Typeid found*/
        typeIdFound = false;
        /*return UA_STATUSCODE_BADDECODINGERROR;*/
    } else {
        typeIdFound = true;
        /* parse the nodeid */
        parseCtx->index = (UA_UInt16)searchTypeIdResult;
        ret = NodeId_decodeJson(&typeId, &UA_TYPES[UA_TYPES_NODEID], ctx, parseCtx, true);
        if(ret != UA_STATUSCODE_GOOD) {
            UA_NodeId_deleteMembers(&typeId);
            return ret;
        }

        /*restore index, ExtensionObject position*/
        parseCtx->index = old_index;
    }

    /* ---Decode the EncodingByte--- */
    if(!typeIdFound)
        return UA_STATUSCODE_BADDECODINGERROR;

    UA_Boolean encodingFound = false;
    /*Search for Encoding*/
    size_t searchEncodingResult = 0;
    ret = lookAheadForKey(UA_JSONKEY_ENCODING, ctx, parseCtx, &searchEncodingResult);

    UA_UInt64 encoding = 0;
    /*If no encoding found it is Structure encoding*/
    if(ret == UA_STATUSCODE_GOOD) { /*FOUND*/
        encodingFound = true;
        char *extObjEncoding = (char*)(ctx->pos + parseCtx->tokenArray[searchEncodingResult].start);
        size_t size = (size_t)(parseCtx->tokenArray[searchEncodingResult].end 
                               - parseCtx->tokenArray[searchEncodingResult].start);
        atoiUnsigned(extObjEncoding, size, &encoding);
    }
        
    const UA_DataType *typeOfBody = UA_findDataType(&typeId);
        
    if(encoding == 0 || typeOfBody != NULL) // <MASK>
    return ret;
}