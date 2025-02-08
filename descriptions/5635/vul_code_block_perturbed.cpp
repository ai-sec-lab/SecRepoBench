SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkISize kernelSize;
    kernelSize.fWidth = inputBuffer.readInt();
    kernelSize.fHeight = inputBuffer.readInt();
    const int count = inputBuffer.getArrayCount();

    const int64_t kernelArea = sk_64_mul(kernelSize.width(), kernelSize.height());
    if (!inputBuffer.validate(kernelArea == count)) {
        return nullptr;
    }
    SkAutoSTArray<16, SkScalar> kernel(count);
    if (!inputBuffer.readScalarArray(kernel.get(), count)) {
        return nullptr;
    }
    SkScalar gain = inputBuffer.readScalar();
    SkScalar bias = inputBuffer.readScalar();
    SkIPoint kernelOffset;
    kernelOffset.fX = inputBuffer.readInt();
    kernelOffset.fY = inputBuffer.readInt();
    TileMode tileMode = (TileMode)inputBuffer.readInt();
    bool convolveAlpha = inputBuffer.readBool();