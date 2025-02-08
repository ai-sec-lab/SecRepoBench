if(statusdecodeStatus != UA_STATUSCODE_GOOD) {
            /* TYPEID not found, abort */
            return UA_STATUSCODE_BADENCODINGERROR;
        }

        /* parse the nodeid */
        /*for restore*/
        UA_UInt16 index = parseCtx->index;
        parseCtx->index = (UA_UInt16)searchTypeIdResult;
        statusdecodeStatus = NodeId_decodeJson(&typeId, &UA_TYPES[UA_TYPES_NODEID], ctx, parseCtx, true);
        if(statusdecodeStatus != UA_STATUSCODE_GOOD)
            return statusdecodeStatus;
        
        /*restore*/
        parseCtx->index = index;
        const UA_DataType *typeOfBody = UA_findDataType(&typeId);
        if(!typeOfBody) {
            /*dont decode body: 1. save as bytestring, 2. jump over*/
            dst->encoding = UA_EXTENSIONOBJECT_ENCODED_BYTESTRING;
            UA_NodeId_copy(&typeId, &dst->content.encoded.typeId);
            
            /*Check if Object in Extentionobject*/
            if(getJsmnType(parseCtx) != JSMN_OBJECT) {
                UA_NodeId_clear(&typeId);
                return UA_STATUSCODE_BADDECODINGERROR;
            }
            
            /*Search for Body to save*/
            size_t searchBodyResult = 0;
            statusdecodeStatus = lookAheadForKey(UA_JSONKEY_BODY, ctx, parseCtx, &searchBodyResult);
            if(statusdecodeStatus != UA_STATUSCODE_GOOD) {
                /*No Body*/
                UA_NodeId_clear(&typeId);
                return UA_STATUSCODE_BADDECODINGERROR;
            }
            
            if(searchBodyResult >= (size_t)parseCtx->tokenCount) {
                /*index not in Tokenarray*/
                UA_NodeId_clear(&typeId);
                return UA_STATUSCODE_BADDECODINGERROR;
            }

            /* Get the size of the Object as a string, not the Object key count! */
            UA_Int64 sizeOfJsonString =(parseCtx->tokenArray[searchBodyResult].end -
                    parseCtx->tokenArray[searchBodyResult].start);
            
            char* bodyJsonString = (char*)(ctx->pos + parseCtx->tokenArray[searchBodyResult].start);
            
            if(sizeOfJsonString <= 0) {
                UA_NodeId_clear(&typeId);
                return UA_STATUSCODE_BADDECODINGERROR;
            }
            
            /* Save encoded as bytestring. */
            statusdecodeStatus = UA_ByteString_allocBuffer(&dst->content.encoded.body, (size_t)sizeOfJsonString);
            if(statusdecodeStatus != UA_STATUSCODE_GOOD) {
                UA_NodeId_clear(&typeId);
                return statusdecodeStatus;
            }

            memcpy(dst->content.encoded.body.data, bodyJsonString, (size_t)sizeOfJsonString);
            
            size_t tokenAfteExtensionObject = 0;
            statusdecodeStatus = jumpOverObject(ctx, parseCtx, &tokenAfteExtensionObject);
            if(statusdecodeStatus != UA_STATUSCODE_GOOD) {
                UA_NodeId_clear(&typeId);
                UA_ByteString_clear(&dst->content.encoded.body);
                return UA_STATUSCODE_BADDECODINGERROR;
            }
            
            parseCtx->index = (UA_UInt16)tokenAfteExtensionObject;
            
            return UA_STATUSCODE_GOOD;
        }
        
        /*Type id not used anymore, typeOfBody has type*/
        UA_NodeId_clear(&typeId);
        
        /*Set Found Type*/
        dst->content.decoded.type = typeOfBody;
        dst->encoding = UA_EXTENSIONOBJECT_DECODED;
        
        if(searchTypeIdResult != 0) {
            dst->content.decoded.data = UA_new(typeOfBody);
            if(!dst->content.decoded.data)
                return UA_STATUSCODE_BADOUTOFMEMORY;

            UA_NodeId typeId_dummy;
            DecodeEntry entries[2] = {
                {UA_JSONKEY_TYPEID, &typeId_dummy, (decodeJsonSignature) NodeId_decodeJson, false, NULL},
                {UA_JSONKEY_BODY, dst->content.decoded.data, (decodeJsonSignature) decodeJsonJumpTable[typeOfBody->typeKind], false, typeOfBody}
            };

            return decodeFields(ctx, parseCtx, entries, 2);
        } else {
           return UA_STATUSCODE_BADDECODINGERROR;
        }