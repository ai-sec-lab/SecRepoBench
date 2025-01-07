
    SkASSERT(count >= 0);
    fCount = count;
    this->shrinkToFit(sizeOfT);
    if (count > 0 && src != nullptr) {
        memcpy(fStorage, src, this->size_bytes(sizeOfT));
    }
