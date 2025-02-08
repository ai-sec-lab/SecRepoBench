sk_sp<SkFlattenable> SkMatrixConvolutionImageFilter::CreateProc(SkReadBuffer& inputBuffer) {
    // <MASK>

    return Make(kernelSize, kernel.get(), gain, bias, kernelOffset, tileMode,
                convolveAlpha, common.getInput(0), &common.cropRect());
}