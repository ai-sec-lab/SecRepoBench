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
                    add_line_to_segment(pts[1], segments);
                }
                break;
            }
            case SkPath::kQuad_Verb:
                if (!SkPathPriv::AllPointsEq(pts, 3)) {
                    m.mapPoints(pts, 3);
                    update_degenerate_test(&degenerateData, pts[1]);
                    update_degenerate_test(&degenerateData, pts[2]);
                    add_quad_segment(pts, segments);
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
                        add_quad_segment(quadPts + 2*i, segments);
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
                    add_cubic_segments(pts, dir, segments);
                }
                break;
            }
            case SkPath::kDone_Verb:
                if (degenerateData.isDegenerate()) {
                    return false;
                } else {
                    return compute_vectors(segments, fanPt, dir, vCount, iCount);
                }
            default:
                break;
        }
    }