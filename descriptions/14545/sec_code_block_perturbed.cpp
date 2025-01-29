if (i.fType == *fContext.fUInt_Type) {
        integerliteral = ConstantType::kUInt;
    } else if (i.fType == *fContext.fShort_Type || i.fType == *fContext.fByte_Type) {
        integerliteral = ConstantType::kShort;
    } else if (i.fType == *fContext.fUShort_Type || i.fType == *fContext.fUByte_Type) {
        integerliteral = ConstantType::kUShort;
    } else {
        SkASSERT(false);
    }