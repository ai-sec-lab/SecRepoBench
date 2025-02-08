int SkSwizzler::onSetSampleX(int samplingFactor) {
    SkASSERT(samplingFactor > 0);

    fSampleX = samplingFactor;
    fSrcOffsetUnits = (get_start_coord(samplingFactor) + fSrcOffset) * fSrcBPP;
    fDstOffsetBytes = (fDstOffset / samplingFactor) * fDstBPP;
    fSwizzleWidth = get_scaled_dimension(fSrcWidth, samplingFactor);
    fAllocatedWidth = get_scaled_dimension(fDstWidth, samplingFactor);

    // <MASK>
}