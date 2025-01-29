{
        /*This value is 0 if the body is Structure encoded as a JSON object (see 5.4.6).*/
        /* Found a valid type and it is structure encoded so it can be unwrapped */
        if (typeOfBody == NULL)
            return UA_STATUSCODE_BADDECODINGERROR;

        variantData->type = typeOfBody;

        /* Allocate memory for type*/
        variantData->data = UA_new(variantData->type);
        if(!variantData->data) {
            UA_NodeId_deleteMembers(&typeId);
            return UA_STATUSCODE_BADOUTOFMEMORY;
        }

        /* Decode the content */
        UA_NodeId nodeIddummy;
        DecodeEntry entries[3] =
            {
             {UA_JSONKEY_TYPEID, &nodeIddummy, (decodeJsonSignature) NodeId_decodeJson, false, NULL},
             {UA_JSONKEY_BODY, variantData->data,
              (decodeJsonSignature) decodeJsonJumpTable[variantData->type->typeKind], false, NULL},
             {UA_JSONKEY_ENCODING, NULL, NULL, false, NULL}};

        ret = decodeFields(ctx, parseCtx, entries, encodingFound ? 3:2, typeOfBody);
        if(ret != UA_STATUSCODE_GOOD) {
            UA_free(variantData->data);
            variantData->data = NULL;
        }
    } else if(encoding == 1 || encoding == 2 || typeOfBody == NULL) {
        UA_NodeId_deleteMembers(&typeId);
            
        /* decode as ExtensionObject */
        variantData->type = &UA_TYPES[UA_TYPES_EXTENSIONOBJECT];

        /* Allocate memory for extensionobject*/
        variantData->data = UA_new(variantData->type);
        if(!variantData->data)
            return UA_STATUSCODE_BADOUTOFMEMORY;

        /* decode: Does not move tokenindex. */
        ret = DECODE_DIRECT_JSON(variantData->data, ExtensionObject);
        if(ret != UA_STATUSCODE_GOOD) {
            UA_free(variantData->data);
            variantData->data = NULL;
        }
    } else {
        /*no recognized encoding type*/
        return UA_STATUSCODE_BADDECODINGERROR;
    }