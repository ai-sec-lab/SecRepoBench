void SkTDStorage::assign(const void* source, int count, size_t sizeOfT) {
    SkASSERT(count >= 0);
    fCount = count;
    this->shrinkToFit(sizeOfT);
    if (count > 0 && source != nullptr) {
        memcpy(fStorage, source, this->size_bytes(sizeOfT));
    }
}