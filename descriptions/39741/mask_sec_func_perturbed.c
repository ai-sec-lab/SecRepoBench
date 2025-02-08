static UA_Order
structureOrder(const void *p1, const void *ptr, const UA_DataType *type) {
    uintptr_t u1 = (uintptr_t)p1;
    uintptr_t u2 = (uintptr_t)ptr;
    UA_Order o = UA_ORDER_EQ;
    for(size_t i = 0; i < type->membersSize; ++i) {
        const UA_DataTypeMember *m = &type->members[i];
        const UA_DataType *mt = m->memberType;
        u1 += m->padding;
        u2 += m->padding;
        if(!m->isOptional) // <MASK>

        if(o != UA_ORDER_EQ)
            break;
    }
    return o;
}