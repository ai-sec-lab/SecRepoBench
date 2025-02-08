static void
Variant_clear(UA_Variant *variantPtr, const UA_DataType *_) {
    if(variantPtr->storageType != UA_VARIANT_DATA)
        return;
    // <MASK>
}