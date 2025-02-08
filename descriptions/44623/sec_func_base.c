DECODE_JSON(ExtensionObject) {
    ALLOW_NULL;
    CHECK_OBJECT;

    /* Search for Encoding */
    size_t searchEncodingResult = 0;
    status ret = lookAheadForKey(UA_JSONKEY_ENCODING, ctx, parseCtx, &searchEncodingResult);
    
    /* If no encoding found it is structure encoding */
    if(ret != UA_STATUSCODE_GOOD) {
        UA_NodeId typeId;
        UA_NodeId_init(&typeId);

        size_t searchTypeIdResult = 0;
        ret = lookAheadForKey(UA_JSONKEY_TYPEID, ctx, parseCtx, &searchTypeIdResult);
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
    } else { /* UA_JSONKEY_ENCODING found */
        /*Parse the encoding*/
        UA_UInt64 encoding = 0;
        char *extObjEncoding = (char*)(ctx->pos + parseCtx->tokenArray[searchEncodingResult].start);
        size_t size = (size_t)(parseCtx->tokenArray[searchEncodingResult].end - parseCtx->tokenArray[searchEncodingResult].start);
        atoiUnsigned(extObjEncoding, size, &encoding);

        if(encoding == 1) {
            /* BYTESTRING in Json Body */
            dst->encoding = UA_EXTENSIONOBJECT_ENCODED_BYTESTRING;
            UA_UInt16 encodingTypeJson;
            DecodeEntry entries[3] = {
                {UA_JSONKEY_ENCODING, &encodingTypeJson, (decodeJsonSignature)UInt16_decodeJson, false, NULL},
                {UA_JSONKEY_BODY, &dst->content.encoded.body, (decodeJsonSignature)String_decodeJson, false, NULL},
                {UA_JSONKEY_TYPEID, &dst->content.encoded.typeId, (decodeJsonSignature)NodeId_decodeJson, false, NULL}
            };
            return decodeFields(ctx, parseCtx, entries, 3);
        } else if(encoding == 2) {
            /* XmlElement in Json Body */
            dst->encoding = UA_EXTENSIONOBJECT_ENCODED_XML;
            UA_UInt16 encodingTypeJson;
            DecodeEntry entries[3] = {
                {UA_JSONKEY_ENCODING, &encodingTypeJson, (decodeJsonSignature)UInt16_decodeJson, false, NULL},
                {UA_JSONKEY_BODY, &dst->content.encoded.body, (decodeJsonSignature)String_decodeJson, false, NULL},
                {UA_JSONKEY_TYPEID, &dst->content.encoded.typeId, (decodeJsonSignature)NodeId_decodeJson, false, NULL}
            };
            return decodeFields(ctx, parseCtx, entries, 3);
        } else {
            return UA_STATUSCODE_BADDECODINGERROR;
        }
    }
    return UA_STATUSCODE_BADNOTIMPLEMENTED;
}