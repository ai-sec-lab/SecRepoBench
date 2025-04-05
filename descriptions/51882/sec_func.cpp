void SkTDStorage::assign(const void* src, int count, size_t sizeOfT) {
    SkASSERT(count >= 0);
    fCount = count;
    this->shrinkToFit(sizeOfT);
    if (count > 0 && src != nullptr) {
        memcpy(fStorage, src, this->size_bytes(sizeOfT));
    }
}