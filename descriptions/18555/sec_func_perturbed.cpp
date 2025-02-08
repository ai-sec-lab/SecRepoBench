bool SkDescriptor::isValid() const {
    uint32_t entryCount = fCount;
    size_t lengthRemaining = this->fLength;
    if (lengthRemaining < sizeof(SkDescriptor)) {
        return false;
    }
    lengthRemaining -= sizeof(SkDescriptor);
    size_t offset = sizeof(SkDescriptor);

    while (lengthRemaining > 0 && entryCount > 0) {
        if (lengthRemaining < sizeof(Entry)) {
            return false;
        }
        lengthRemaining -= sizeof(Entry);

        const Entry* entry = (const Entry*)(reinterpret_cast<const char*>(this) + offset);

        if (lengthRemaining < entry->fLen) {
            return false;
        }
        lengthRemaining -= entry->fLen;

        // rec tags are always a known size.
        if (entry->fTag == kRec_SkDescriptorTag && entry->fLen != sizeof(SkScalerContextRec)) {
            return false;
        }

        offset += sizeof(Entry) + entry->fLen;
        entryCount--;
    }
    return lengthRemaining == 0 && entryCount == 0;
}