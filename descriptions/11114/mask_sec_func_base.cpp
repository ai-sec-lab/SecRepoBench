int SkSwizzler::onSetSampleX(int sampleX) {
    SkASSERT(sampleX > 0);

    fSampleX = sampleX;
    // <MASK>

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