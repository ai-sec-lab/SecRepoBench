if(variantPtr->type && variantPtr->data > UA_EMPTY_ARRAY_SENTINEL) {
        if(variantPtr->arrayLength == 0)
            variantPtr->arrayLength = 1;
        UA_Array_delete(variantPtr->data, variantPtr->arrayLength, variantPtr->type);
    }
    if((void*)variantPtr->arrayDimensions > UA_EMPTY_ARRAY_SENTINEL)
        UA_free(variantPtr->arrayDimensions);