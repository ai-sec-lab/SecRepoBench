if (i.fType == *fContext.fUInt_Type) {
        type = ConstantType::kUInt;
    } else if (i.fType == *fContext.fShort_Type || i.fType == *fContext.fByte_Type) {
        type = ConstantType::kShort;
    } else if (i.fType == *fContext.fUShort_Type || i.fType == *fContext.fUByte_Type) {
        type = ConstantType::kUShort;
    } else {
        SkASSERT(false);
    }