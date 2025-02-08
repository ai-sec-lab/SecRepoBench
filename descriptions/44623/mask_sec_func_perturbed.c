DECODE_JSON(ExtensionObject) {
    ALLOW_NULL;
    CHECK_OBJECT;

    /* Search for Encoding */
    size_t searchEncodingResult = 0;
    status statusdecodeStatus = lookAheadForKey(UA_JSONKEY_ENCODING, ctx, parseCtx, &searchEncodingResult);
    
    /* If no encoding found it is structure encoding */
    if(statusdecodeStatus != UA_STATUSCODE_GOOD) {
        UA_NodeId typeId;
        UA_NodeId_init(&typeId);

        size_t searchTypeIdResult = 0;
        statusdecodeStatus = lookAheadForKey(UA_JSONKEY_TYPEID, ctx, parseCtx, &searchTypeIdResult);
        // <MASK>
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