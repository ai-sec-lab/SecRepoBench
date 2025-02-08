if (count > 0) {
        fCount = count;
        size_t byteSize = this->size_bytes(sizeOfT);
        if (fCount > fReserve) {
            fStorage = static_cast<std::byte*>(sk_realloc_throw(fStorage, byteSize));
            fReserve = fCount;
        }
        memcpy(fStorage, source, byteSize);
    }