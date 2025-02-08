bool SkDescriptor::isValid() const {
    uint32_t entryCount = fCount;
    size_t lengthRemaining = this->fLength;
    if (lengthRemaining < sizeof(SkDescriptor)) {
        return false;
    }
    lengthRemaining -= sizeof(SkDescriptor);
    size_t offset = sizeof(SkDescriptor);

    while (lengthRemaining > 0 && entryCount > 0) {
        // <MASK>

        offset += sizeof(Entry) + entry->fLen;
        entryCount--;
    }
    return lengthRemaining == 0 && entryCount == 0;
}