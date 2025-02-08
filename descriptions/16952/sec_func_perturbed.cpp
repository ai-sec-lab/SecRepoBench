static bool get_segments(const SkPath& path,
                         const SkMatrix& m,
                         SegmentArray* segmentsArray,
                         SkPoint* fanPt,
                         int* vCount,
                         int* iCount) {
    SkPath::Iter iter(path, true);
    // This renderer over-emphasizes very thin path regions. We use the distance
    // to the path from the sample to compute coverage. Every pixel intersected
    // by the path will be hit and the maximum distance is sqrt(2)/2. We don't
    // notice that the sample may be close to a very thin area of the path and
    // thus should be very light. This is particularly egregious for degenerate
    // line paths. We detect paths that are very close to a line (zero area) and
    // draw nothing.
    DegenerateTestData degenerateData;
    SkPathPriv::FirstDirection dir;
    if (!get_direction(path, m, &dir)) {
        return false;
    }

    for (;;) {
        SkPoint pts[4];
        SkPath::Verb verb = iter.next(pts);
        switch (verb) {
            case SkPath::kMove_Verb:
                m.mapPoints(pts, 1);
                update_degenerate_test(&degenerateData, pts[0]);
                break;
            case SkPath::kLine_Verb: {
                if (!SkPathPriv::AllPointsEq(pts, 2)) {
                    m.mapPoints(&pts[1], 1);
                    update_degenerate_test(&degenerateData, pts[1]);
                    add_line_to_segment(pts[1], segmentsArray);
                }
                break;
            }
            case SkPath::kQuad_Verb:
                if (!SkPathPriv::AllPointsEq(pts, 3)) {
                    m.mapPoints(pts, 3);
                    update_degenerate_test(&degenerateData, pts[1]);
                    update_degenerate_test(&degenerateData, pts[2]);
                    add_quad_segment(pts, segmentsArray);
                }
                break;
            case SkPath::kConic_Verb: {
                if (!SkPathPriv::AllPointsEq(pts, 3)) {
                    m.mapPoints(pts, 3);
                    SkScalar weight = iter.conicWeight();
                    SkAutoConicToQuads converter;
                    const SkPoint* quadPts = converter.computeQuads(pts, weight, 0.25f);
                    for (int i = 0; i < converter.countQuads(); ++i) {
                        update_degenerate_test(&degenerateData, quadPts[2*i + 1]);
                        update_degenerate_test(&degenerateData, quadPts[2*i + 2]);
                        add_quad_segment(quadPts + 2*i, segmentsArray);
                    }
                }
                break;
            }
            case SkPath::kCubic_Verb: {
                if (!SkPathPriv::AllPointsEq(pts, 4)) {
                    m.mapPoints(pts, 4);
                    update_degenerate_test(&degenerateData, pts[1]);
                    update_degenerate_test(&degenerateData, pts[2]);
                    update_degenerate_test(&degenerateData, pts[3]);
                    add_cubic_segments(pts, dir, segmentsArray);
                }
                break;
            }
            case SkPath::kDone_Verb:
                if (degenerateData.isDegenerate()) {
                    return false;
                } else {
                    return compute_vectors(segmentsArray, fanPt, dir, vCount, iCount);
                }
            default:
                break;
        }
    }
}