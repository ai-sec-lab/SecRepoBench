static status
ExtensionObject_decodeBinaryContent(UA_ExtensionObject *dst, const UA_NodeId *typeId) {
    /* Lookup the datatype */
    const UA_DataType *type = UA_findDataTypeByBinary(typeId);

    /* Unknown type, just take the binary content */
    if(!type) {
        dst->encoding = UA_EXTENSIONOBJECT_ENCODED_BYTESTRING;
        UA_NodeId_copy(typeId, &dst->content.encoded.typeId);
        return ByteString_decodeBinary(&dst->content.encoded.body);
    }

    /* Allocate memory */
    dst->content.decoded.data = UA_new(type);
    if(!dst->content.decoded.data)
        return UA_STATUSCODE_BADOUTOFMEMORY;

    /* Jump over the length field (TODO: check if the decoded length matches) */
    g_pos += 4;
        
    /* Decode */
    dst->encoding = UA_EXTENSIONOBJECT_DECODED;
    dst->content.decoded.type = type;
    size_t decode_index = type->builtin ? type->typeIndex : UA_BUILTIN_TYPES_COUNT;
    return decodeBinaryJumpTable[decode_index](dst->content.decoded.data, type);
}