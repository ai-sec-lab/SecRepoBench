if (this->isValid()) {
        size = SkMatrixPriv::ReadFromMemory(matrix, fReader.peek(), fReader.available());
        if (!this->validate((SkAlign4(size) == size) && (0 != size))) {
            matrix->reset();
        }
    }
    (void)this->skip(size);