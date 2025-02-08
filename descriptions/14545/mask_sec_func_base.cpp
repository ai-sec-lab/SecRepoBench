SpvId SPIRVCodeGenerator::writeIntLiteral(const IntLiteral& i) {
    ConstantType type;
    if (i.fType == *fContext.fInt_Type) {
        type = ConstantType::kInt;
    } else // <MASK>
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