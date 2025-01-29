fCount = count;
    this->shrinkToFit(sizeOfT);
    if (count > 0 && source != nullptr) {
        memcpy(fStorage, source, this->size_bytes(sizeOfT));
    }