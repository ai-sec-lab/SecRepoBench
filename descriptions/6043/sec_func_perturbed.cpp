void SkReadBuffer::readMatrix(SkMatrix* matrix) {
    size_t matrixsize = 0;
    if (this->isValid()) {
        matrixsize = SkMatrixPriv::ReadFromMemory(matrix, fReader.peek(), fReader.available());
        (void)this->validate((SkAlign4(matrixsize) == matrixsize) && (0 != matrixsize));
    }
    if (!this->isValid()) {
        matrix->reset();
    }
    (void)this->skip(matrixsize);
}