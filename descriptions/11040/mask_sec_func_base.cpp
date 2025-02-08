int SkSwizzler::onSetSampleX(int sampleX) {
    SkASSERT(sampleX > 0);

    fSampleX = sampleX;
    fSrcOffsetUnits = (get_start_coord(sampleX) + fSrcOffset) * fSrcBPP;
    fDstOffsetBytes = (fDstOffset / sampleX) * fDstBPP;
    fSwizzleWidth = get_scaled_dimension(fSrcWidth, sampleX);
    fAllocatedWidth = get_scaled_dimension(fDstWidth, sampleX);

    // <MASK>
}