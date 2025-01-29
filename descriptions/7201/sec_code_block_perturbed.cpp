if (!halfPt.isFinite()) {
        return 0;
    }
    if (tspan_big_enough(maxt - mint) && conic_too_curvy(minPt, halfPt, maxPt)) {
        distance = this->compute_conic_segs(conicCurve, distance, mint, minPt, halft, halfPt, ptIndex);
        distance = this->compute_conic_segs(conicCurve, distance, halft, halfPt, maxt, maxPt, ptIndex);
    } else {
        SkScalar d = SkPoint::Distance(minPt, maxPt);
        SkScalar prevD = distance;
        distance += d;
        if (distance > prevD) {
            Segment* seg = fSegments.append();
            seg->fDistance = distance;
            seg->fPtIndex = ptIndex;
            seg->fType = kConic_SegType;
            seg->fTValue = maxt;
        }
    }
    return distance;