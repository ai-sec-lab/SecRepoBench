if(p->type && p->data > UA_EMPTY_ARRAY_SENTINEL) {
        if(p->arrayLength == 0)
            p->arrayLength = 1;
        UA_Array_delete(p->data, p->arrayLength, p->type);
    }
    if((void*)p->arrayDimensions > UA_EMPTY_ARRAY_SENTINEL)
        UA_free(p->arrayDimensions);