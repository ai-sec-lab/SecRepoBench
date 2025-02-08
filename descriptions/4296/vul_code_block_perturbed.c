/* Unknown type, just take the binary content */
    if(!type) {
        dst->encoding = UA_EXTENSIONOBJECT_ENCODED_BYTESTRING;
        dst->content.encoded.typeId = *typeIdentifier;
        return ByteString_decodeBinary(&dst->content.encoded.body);
    }

    /* Allocate memory */