bool SkDescriptor::isValid() const {
    uint32_t count = fCount;
    size_t lengthRemaining = this->fLength;
    if (lengthRemaining < sizeof(SkDescriptor)) {
        return false;
    }
    lengthRemaining -= sizeof(SkDescriptor);
    size_t offset = sizeof(SkDescriptor);

    while (lengthRemaining > 0 && count > 0) {
        // <MASK>

        offset += sizeof(Entry) + entry->fLen;
        count--;
    }
    return lengthRemaining == 0 && count == 0;
}