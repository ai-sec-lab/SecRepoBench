SpvId SPIRVCodeGenerator::writeIntLiteral(const IntLiteral& i) {
    ConstantType integerliteral;
    if (i.fType == *fContext.fInt_Type) {
        integerliteral = ConstantType::kInt;
    } else if (i.fType == *fContext.fUInt_Type) {
        integerliteral = ConstantType::kUInt;
    } else if (i.fType == *fContext.fShort_Type || i.fType == *fContext.fByte_Type) {
        integerliteral = ConstantType::kShort;
    } else if (i.fType == *fContext.fUShort_Type || i.fType == *fContext.fUByte_Type) {
        integerliteral = ConstantType::kUShort;
    } else {
        SkASSERT(false);
    }
    std::pair<ConstantValue, ConstantType> key(i.fValue, integerliteral);
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