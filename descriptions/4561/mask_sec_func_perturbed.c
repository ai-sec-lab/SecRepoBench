static UA_StatusCode
writeArrayDimensionsAttribute(UA_Server *server, UA_Session *session,
                              UA_VariableNode *variableNode, const UA_VariableTypeNode *type,
                              size_t arrayDimensionsSize, UA_UInt32 *arrayDimensions) {
    UA_assert(variableNode != NULL);
    UA_assert(type != NULL);

    /* If this is a variabletype, there must be no instances or subtypes of it
     * when we do the change */
    if(variableNode->nodeClass == UA_NODECLASS_VARIABLETYPE &&
       UA_Node_hasSubTypeOrInstances((UA_Node*)variableNode)) {
        UA_LOG_INFO(server->config.logger, UA_LOGCATEGORY_SERVER,
                    "Cannot change a variable type with existing instances");
        return UA_STATUSCODE_BADINTERNALERROR;
    }

    /* Check that the array dimensions match with the valuerank */
    if(!compatibleValueRankArrayDimensions(variableNode->valueRank, arrayDimensionsSize)) {
        UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SERVER,
                     "The current value rank does not match the new array dimensions");
        return UA_STATUSCODE_BADTYPEMISMATCH;
    }

    /* Check if the array dimensions match with the wildcards in the
     * variabletype (dimension length 0) */
    if(type->arrayDimensions &&
       !compatibleArrayDimensions(type->arrayDimensionsSize, type->arrayDimensions,
                                  arrayDimensionsSize, arrayDimensions)) {
       UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SERVER,
                    "Array dimensions in the variable type do not match");
       return UA_STATUSCODE_BADTYPEMISMATCH;
    }

    /* Check if the current value is compatible with the array dimensions */
    UA_DataValue value;
    UA_DataValue_init(&value);
    UA_StatusCode retval = readValueAttribute(server, session, variableNode, &value);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    if(value.hasValue) {
        if(!compatibleValueArrayDimensions(&value.value, arrayDimensionsSize, arrayDimensions))
            retval = UA_STATUSCODE_BADTYPEMISMATCH;
        UA_DataValue_deleteMembers(&value);
        if(retval != UA_STATUSCODE_GOOD) {
            UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SERVER,
                         "Array dimensions in the current value do not match");
            return retval;
        }
    }

    /* Ok, apply */
    UA_UInt32 *oldArrayDimensions = variableNode->arrayDimensions;
    // <MASK>
    variableNode->arrayDimensionsSize = arrayDimensionsSize;
    return UA_STATUSCODE_GOOD;
}