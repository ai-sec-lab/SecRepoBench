static UA_StatusCode
writeValueAttributeWithRange(UA_VariableNode *node, const UA_DataValue *value,
                             const UA_NumericRange *rangeptr) {
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
    // <MASK>
}