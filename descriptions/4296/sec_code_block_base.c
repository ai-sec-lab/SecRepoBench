/* Unknown type, just take the binary content */
    if(!type) {
        dst->encoding = UA_EXTENSIONOBJECT_ENCODED_BYTESTRING;
        UA_NodeId_copy(typeId, &dst->content.encoded.typeId);
        return ByteString_decodeBinary(&dst->content.encoded.body);
    }

    /* Allocate memory */