SkScalar SkPathMeasure::compute_conic_segs(const SkConic& conic, SkScalar distance,
                                           int mint, const SkPoint& minPt,
                                           int maxt, const SkPoint& maxPt, unsigned ptIndex) {
    int halft = (mint + maxt) >> 1;
    SkPoint halfPt = conic.evalAt(tValue2Scalar(halft));
    // <MASK>
}