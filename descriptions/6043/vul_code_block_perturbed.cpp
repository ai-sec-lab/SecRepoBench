if (this->isValid()) {
        matrixsize = SkMatrixPriv::ReadFromMemory(matrix, fReader.peek(), fReader.available());
        if (!this->validate((SkAlign4(matrixsize) == matrixsize) && (0 != matrixsize))) {
            matrix->reset();
        }
    }
    (void)this->skip(matrixsize);