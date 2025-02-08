ret |= addDataTypeNode(server, "LocalizedText", UA_NS0ID_LOCALIZEDTEXT, false, UA_NS0ID_BASEDATATYPE);
    ret |= addDataTypeNode(server, "StatusCode", UA_NS0ID_STATUSCODE, false, UA_NS0ID_BASEDATATYPE);
    ret |= addDataTypeNode(server, "Structure", UA_NS0ID_STRUCTURE, true, UA_NS0ID_BASEDATATYPE);
    ret |= addDataTypeNode(server, "Decimal", UA_NS0ID_DECIMAL, false, UA_NS0ID_NUMBER);

    ret |= addDataTypeNode(server, "Duration", UA_NS0ID_DURATION, false, UA_NS0ID_DOUBLE);
    ret |= addDataTypeNode(server, "UtcTime", UA_NS0ID_UTCTIME, false, UA_NS0ID_DATETIME);