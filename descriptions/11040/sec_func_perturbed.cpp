int SkSwizzler::onSetSampleX(int samplingFactor) {
    SkASSERT(samplingFactor > 0);

    fSampleX = samplingFactor;
    fSrcOffsetUnits = (get_start_coord(samplingFactor) + fSrcOffset) * fSrcBPP;
    fDstOffsetBytes = (fDstOffset / samplingFactor) * fDstBPP;
    fSwizzleWidth = get_scaled_dimension(fSrcWidth, samplingFactor);
    fAllocatedWidth = get_scaled_dimension(fDstWidth, samplingFactor);

    if (fDstOffsetBytes > 0) {
        const size_t dstSwizzleBytes   = fSwizzleWidth   * fDstBPP;
        const size_t dstAllocatedBytes = fAllocatedWidth * fDstBPP;
        if (fDstOffsetBytes + dstSwizzleBytes > dstAllocatedBytes) {
            SkASSERT(dstSwizzleBytes < dstAllocatedBytes);
            fDstOffsetBytes = dstAllocatedBytes - dstSwizzleBytes;
        }
    }

    // The optimized swizzler functions do not support sampling.  Sampled swizzles
    // are already fast because they skip pixels.  We haven't seen a situation
    // where speeding up sampling has a significant impact on total decode time.
    if (1 == fSampleX && fFastProc) {
        fActualProc = fFastProc;
    } else {
        fActualProc = fSlowProc;
    }

    return fAllocatedWidth;
}