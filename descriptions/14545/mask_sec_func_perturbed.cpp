SpvId SPIRVCodeGenerator::writeIntLiteral(const IntLiteral& i) {
    ConstantType integerliteral;
    if (i.fType == *fContext.fInt_Type) {
        integerliteral = ConstantType::kInt;
    } else // <MASK>
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