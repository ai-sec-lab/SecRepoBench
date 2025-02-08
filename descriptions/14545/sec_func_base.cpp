SpvId SPIRVCodeGenerator::writeIntLiteral(const IntLiteral& i) {
    ConstantType type;
    if (i.fType == *fContext.fInt_Type) {
        type = ConstantType::kInt;
    } else if (i.fType == *fContext.fUInt_Type) {
        type = ConstantType::kUInt;
    } else if (i.fType == *fContext.fShort_Type || i.fType == *fContext.fByte_Type) {
        type = ConstantType::kShort;
    } else if (i.fType == *fContext.fUShort_Type || i.fType == *fContext.fUByte_Type) {
        type = ConstantType::kUShort;
    } else {
        SkASSERT(false);
    }
    std::pair<ConstantValue, ConstantType> key(i.fValue, type);
    auto entry = fNumberConstants.find(key);
    if (entry == fNumberConstants.end()) {
        SpvId result = this->nextId();
        this->writeInstruction(SpvOpConstant, this->getType(i.fType), result, (SpvId) i.fValue,
                               fConstantBuffer);
        fNumberConstants[key] = result;
        return result;
    }
    return entry->second;
}