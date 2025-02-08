sk_sp<SkFlattenable> SkMatrixConvolutionImageFilter::CreateProc(SkReadBuffer& buffer) {
    // <MASK>

    return Make(kernelSize, kernel.get(), gain, bias, kernelOffset, tileMode,
                convolveAlpha, common.getInput(0), &common.cropRect());
}