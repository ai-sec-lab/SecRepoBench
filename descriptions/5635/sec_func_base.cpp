sk_sp<SkFlattenable> SkMatrixConvolutionImageFilter::CreateProc(SkReadBuffer& buffer) {
    SkSafeRange safe;

    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);

    SkISize kernelSize;
    kernelSize.fWidth = buffer.readInt();
    kernelSize.fHeight = buffer.readInt();
    const int count = buffer.getArrayCount();

    const int64_t kernelArea = sk_64_mul(kernelSize.width(), kernelSize.height());
    if (!buffer.validate(kernelArea == count)) {
        return nullptr;
    }
    SkAutoSTArray<16, SkScalar> kernel(count);
    if (!buffer.readScalarArray(kernel.get(), count)) {
        return nullptr;
    }
    SkScalar gain = buffer.readScalar();
    SkScalar bias = buffer.readScalar();
    SkIPoint kernelOffset;
    kernelOffset.fX = buffer.readInt();
    kernelOffset.fY = buffer.readInt();

    TileMode tileMode = safe.checkLE<TileMode>(buffer.readInt(), kLast_TileMode);
    bool convolveAlpha = buffer.readBool();

    if (!buffer.validate(safe)) {
        return nullptr;
    }

    return Make(kernelSize, kernel.get(), gain, bias, kernelOffset, tileMode,
                convolveAlpha, common.getInput(0), &common.cropRect());
}