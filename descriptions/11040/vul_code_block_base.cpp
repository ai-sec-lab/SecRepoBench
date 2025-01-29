// The optimized swizzler functions do not support sampling.  Sampled swizzles
    // are already fast because they skip pixels.  We haven't seen a situation
    // where speeding up sampling has a significant impact on total decode time.
    if (1 == fSampleX && fFastProc) {
        fActualProc = fFastProc;
    } else {
        fActualProc = fSlowProc;
    }

    return fAllocatedWidth;