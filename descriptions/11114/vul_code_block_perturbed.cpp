    fSrcOffsetUnits = (get_start_coord(sampleRateX) + fSrcOffset) * fSrcBPP;
    fDstOffsetBytes = (fDstOffset / sampleRateX) * fDstBPP;
    fSwizzleWidth = get_scaled_dimension(fSrcWidth, sampleRateX);
    fAllocatedWidth = get_scaled_dimension(fDstWidth, sampleRateX);
