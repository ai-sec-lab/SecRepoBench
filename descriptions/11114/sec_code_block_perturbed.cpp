fDstOffsetBytes = (fDstOffset / sampleRateX) * fDstBPP;
    fSwizzleWidth = get_scaled_dimension(fSrcWidth, sampleRateX);
    fAllocatedWidth = get_scaled_dimension(fDstWidth, sampleRateX);

    int frameSampleX = sampleRateX;
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