static UA_StatusCode
writeValueAttributeWithRange(UA_VariableNode *node, const UA_DataValue *value,
                             const UA_NumericRange *rangePtr) {
    /* Value on both sides? */
    if(value->status != node->value.data.value.status ||
       !value->hasValue || !node->value.data.value.hasValue)
        return UA_STATUSCODE_BADINDEXRANGEINVALID;

    /* Make scalar a one-entry array for range matching */
    UA_Variant editableValue;
    const UA_Variant *v = &value->value;
    if(UA_Variant_isScalar(&value->value)) {
        editableValue = value->value;
        editableValue.arrayLength = 1;
        v = &editableValue;
    }

    /* Check that the type is an exact match and not only "compatible" */
    if(!node->value.data.value.value.type ||
       !UA_NodeId_equal(&node->value.data.value.value.type->typeId,
                        &v->type->typeId))
        return UA_STATUSCODE_BADTYPEMISMATCH;

    /* Write the value */
    UA_StatusCode retval = UA_Variant_setRangeCopy(&node->value.data.value.value,
                                                   v->data, v->arrayLength, *rangePtr);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;

    /* Write the status and timestamps */
    node->value.data.value.hasStatus = value->hasStatus;
    node->value.data.value.status = value->status;
    node->value.data.value.hasSourceTimestamp = value->hasSourceTimestamp;
    node->value.data.value.sourceTimestamp = value->sourceTimestamp;
    node->value.data.value.hasSourcePicoseconds = value->hasSourcePicoseconds;
    node->value.data.value.sourcePicoseconds = value->sourcePicoseconds;
    return UA_STATUSCODE_GOOD;
}