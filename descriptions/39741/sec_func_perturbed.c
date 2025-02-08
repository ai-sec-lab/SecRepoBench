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
        if(!m->isOptional) {
            if(!m->isArray) {
                o = orderJumpTable[mt->typeKind]((const void *)u1, (const void *)u2, mt);
                u1 += mt->memSize;
                u2 += mt->memSize;
            } else {
                size_t size1 = *(size_t*)u1;
                size_t size2 = *(size_t*)u2;
                u1 += sizeof(size_t);
                u2 += sizeof(size_t);
                o = arrayOrder(*(void* const*)u1, size1, *(void* const*)u2, size2, mt);
                u1 += sizeof(void*);
                u2 += sizeof(void*);
            }
        } else {
            if(!m->isArray) {
                const void *pp1 = *(void* const*)u1;
                const void *pp2 = *(void* const*)u2;
                if(pp1 == pp2) {
                    o = UA_ORDER_EQ;
                } else if(pp1 == NULL) {
                    o = UA_ORDER_LESS;
                } else if(pp2 == NULL) {
                    o = UA_ORDER_MORE;
                } else {
                    o = orderJumpTable[mt->typeKind](pp1, pp2, mt);
                }
            } else {
                size_t sa1 = *(size_t*)u1;
                size_t sa2 = *(size_t*)u2;
                u1 += sizeof(size_t);
                u2 += sizeof(size_t);
                o = arrayOrder(*(void* const*)u1, sa1, *(void* const*)u2, sa2, mt);
            }
            u1 += sizeof(void*);
            u2 += sizeof(void*);
        }

        if(o != UA_ORDER_EQ)
            break;
    }
    return o;
}