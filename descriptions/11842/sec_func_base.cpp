SkPath::Convexity SkPath::internalGetConvexity() const {
    SkPoint         pts[4];
    SkPath::Verb    verb;
    SkPath::Iter    iter(*this, true);
    auto setComputedConvexity = [=](Convexity convexity){
        SkASSERT(kUnknown_Convexity != convexity);
        this->setConvexity(convexity);
        return convexity;
    };

    // Check to see if path changes direction more than three times as quick concave test
    int pointCount = this->countPoints();
    // last moveTo index may exceed point count if data comes from fuzzer (via SkImageFilter)
    if (0 < fLastMoveToIndex && fLastMoveToIndex < pointCount) {
        pointCount = fLastMoveToIndex;
    }
    if (pointCount > 3) {
        const SkPoint* points = fPathRef->points();
        const SkPoint* last = &points[pointCount];
        // only consider the last of the initial move tos
        while (SkPath::kMove_Verb == iter.next(pts, false, false)) {
            ++points;
        }
        --points;
        SkPath::Convexity convexity = Convexicator::BySign(points, (int) (last - points));
        if (SkPath::kConcave_Convexity == convexity) {
            return setComputedConvexity(SkPath::kConcave_Convexity);
        } else if (SkPath::kUnknown_Convexity == convexity) {
            return SkPath::kUnknown_Convexity;
        }
        iter.setPath(*this, true);
    } else if (!this->isFinite()) {
        return kUnknown_Convexity;
    }

    int             contourCount = 0;
    int             count;
    Convexicator    state;
    auto setFail = [=](){
        if (!state.isFinite()) {
            return SkPath::kUnknown_Convexity;
        }
        return setComputedConvexity(SkPath::kConcave_Convexity);
    };

    while ((verb = iter.next(pts, false, false)) != SkPath::kDone_Verb) {
        switch (verb) {
            case kMove_Verb:
                if (++contourCount > 1) {
                    return setComputedConvexity(kConcave_Convexity);
                }
                state.setMovePt(pts[0]);
                count = 0;
                break;
            case kLine_Verb:
                count = 1;
                break;
            case kQuad_Verb:
                // fall through
            case kConic_Verb:
                count = 2;
                break;
            case kCubic_Verb:
                count = 3;
                break;
            case kClose_Verb:
                if (!state.close()) {
                    return setFail();
                }
                count = 0;
                break;
            default:
                SkDEBUGFAIL("bad verb");
                return setComputedConvexity(kConcave_Convexity);
        }
        for (int i = 1; i <= count; i++) {
            if (!state.addPt(pts[i])) {
                return setFail();
            }
        }
    }

    if (this->getFirstDirection() == SkPathPriv::kUnknown_FirstDirection) {
        if (state.getFirstDirection() == SkPathPriv::kUnknown_FirstDirection
                && !this->getBounds().isEmpty()) {
            return setComputedConvexity(state.reversals() < 3 ?
                    kConvex_Convexity : kConcave_Convexity);
        }
        this->setFirstDirection(state.getFirstDirection());
    }
    return setComputedConvexity(kConvex_Convexity);
}