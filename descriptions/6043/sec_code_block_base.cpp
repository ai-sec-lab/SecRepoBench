if (this->isValid()) {
        size = SkMatrixPriv::ReadFromMemory(matrix, fReader.peek(), fReader.available());
        (void)this->validate((SkAlign4(size) == size) && (0 != size));
    }
    if (!this->isValid()) {
        matrix->reset();
    }
    (void)this->skip(size);