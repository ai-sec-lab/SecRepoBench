int SkSwizzler::onSetSampleX(int sampleX) {
    SkASSERT(sampleX > 0);

    fSampleX = sampleX;
    fDstOffsetBytes = (fDstOffset / sampleX) * fDstBPP;
    fSwizzleWidth = get_scaled_dimension(fSrcWidth, sampleX);
    fAllocatedWidth = get_scaled_dimension(fDstWidth, sampleX);

    int frameSampleX = sampleX;
    if (fSrcWidth < fDstWidth) {
        // Although SkSampledCodec adjusted sampleX so that it will never be
        // larger than the width of the image (or subset, if applicable), it
        // doesn't account for the width of a subset frame (i.e. gif). As a
        // result, get_start_coord(sampleX) could result in fSrcOffsetUnits
        // being wider than fSrcWidth. Compute a sampling rate based on the
        // frame width to ensure that fSrcOffsetUnits is sensible.
        frameSampleX = fSrcWidth / fSwizzleWidth;
    }
    fSrcOffsetUnits = (get_start_coord(frameSampleX) + fSrcOffset) * fSrcBPP;

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