int pointCount = this->countPoints();
    // last moveTo index may exceed point count if data comes from fuzzer (via SkImageFilter)
    if (0 < fLastMoveToIndex && fLastMoveToIndex < pointCount) {
        pointCount = fLastMoveToIndex;
    }