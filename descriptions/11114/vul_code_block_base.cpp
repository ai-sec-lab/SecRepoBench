    fSrcOffsetUnits = (get_start_coord(sampleX) + fSrcOffset) * fSrcBPP;
    fDstOffsetBytes = (fDstOffset / sampleX) * fDstBPP;
    fSwizzleWidth = get_scaled_dimension(fSrcWidth, sampleX);
    fAllocatedWidth = get_scaled_dimension(fDstWidth, sampleX);
