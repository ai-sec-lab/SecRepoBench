if(ret != UA_STATUSCODE_GOOD)
            return UA_STATUSCODE_BADENCODINGERROR;

        /* Parse the nodeid */
        UA_UInt16 index = parseCtx->index; /* to restore later */
        parseCtx->index = (UA_UInt16)searchTypeIdResult;
        ret = NodeId_decodeJson(&typeId, &UA_TYPES[UA_TYPES_NODEID], ctx, parseCtx, true);
        if(ret != UA_STATUSCODE_GOOD)
            return ret;
        
        /* Restore the index to the beginning of the object  */
        parseCtx->index = index;
        const UA_DataType *typeOfBody = UA_findDataType(&typeId);
        if(!typeOfBody) {
            /* Dont decode body: 1. save as bytestring, 2. jump over */
            dst->encoding = UA_EXTENSIONOBJECT_ENCODED_BYTESTRING;
            dst->content.encoded.typeId = typeId; /* Move the type NodeId */
            
            /* Check if an object */
            if(getJsmnType(parseCtx) != JSMN_OBJECT)
                return UA_STATUSCODE_BADDECODINGERROR;
            
            /* Search for body to save */
            size_t searchBodyResult = 0;
            ret = lookAheadForKey(UA_JSONKEY_BODY, ctx, parseCtx, &searchBodyResult);
            if(ret != UA_STATUSCODE_GOOD || searchBodyResult >= (size_t)parseCtx->tokenCount)
                return UA_STATUSCODE_BADDECODINGERROR;

            /* Get the size of the Object as a string, not the Object key count! */
            size_t sizeOfJsonString = (size_t)
                (parseCtx->tokenArray[searchBodyResult].end -
                 parseCtx->tokenArray[searchBodyResult].start);
            if(sizeOfJsonString == 0)
                return UA_STATUSCODE_BADDECODINGERROR;
            
            char* bodyJsonString = (char*)(ctx->pos + parseCtx->tokenArray[searchBodyResult].start);
            
            /* Copy body as bytestring. */
            ret = UA_ByteString_allocBuffer(&dst->content.encoded.body, sizeOfJsonString);
            if(ret != UA_STATUSCODE_GOOD)
                return ret;

            memcpy(dst->content.encoded.body.data, bodyJsonString, sizeOfJsonString);
            
            skipObject(parseCtx); /* parseCtx->index is still at the object
                                   * beginning. Skip. */
            return UA_STATUSCODE_GOOD;
        }
        
        /* Type id not used anymore, typeOfBody has type */
        UA_NodeId_clear(&typeId);

        /* Allocate */
        dst->content.decoded.data = UA_new(typeOfBody);
        if(!dst->content.decoded.data)
            return UA_STATUSCODE_BADOUTOFMEMORY;
        
        /* Set Found Type */
        dst->content.decoded.type = typeOfBody;
        dst->encoding = UA_EXTENSIONOBJECT_DECODED;

        /* Decode body */
        DecodeEntry entries[2] = {
            {UA_JSONKEY_TYPEID, NULL, NULL, false, NULL},
            {UA_JSONKEY_BODY, dst->content.decoded.data, (decodeJsonSignature) decodeJsonJumpTable[typeOfBody->typeKind], false, typeOfBody}
        };
        return decodeFields(ctx, parseCtx, entries, 2);