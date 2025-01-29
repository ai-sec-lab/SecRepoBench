size_t oldArrayDimensionsSize = variableNode->arrayDimensionsSize;
    retval = UA_Array_copy(arrayDimensions, arrayDimensionsSize,
                           (void**)&variableNode->arrayDimensions,
                           &UA_TYPES[UA_TYPES_UINT32]);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    UA_Array_delete(oldArrayDimensions, oldArrayDimensionsSize, &UA_TYPES[UA_TYPES_UINT32]);