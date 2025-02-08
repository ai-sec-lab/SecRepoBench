SkPath& SkPath::reverseAddPath(const SkPath& srcPath) {
    // Detect if we're trying to add ourself
    const SkPath* src = &srcPath;
    SkTLazy<SkPath> tmp;
    if (this == src) {
        src = tmp.set(srcPath);
    }

    SkPathRef::Editor ed(&fPathRef, src->fPathRef->countPoints(), src->fPathRef->countVerbs());

    const SkPoint* pts = src->fPathRef->pointsEnd();
    // we will iterator through src's verbs backwards
    const uint8_t* verbs = src->fPathRef->verbsMemBegin(); // points at the last verb
    const uint8_t* verbsEnd = src->fPathRef->verbs(); // points just past the first verb
    const SkScalar* conicWeights = src->fPathRef->conicWeightsEnd();

    bool needMove = true;
    bool needClose = false;
    while (verbs < verbsEnd) {
        uint8_t v = *(verbs++);
        int n = pts_in_verb(v);

        if (needMove) {
            --pts;
            this->moveTo(pts->fX, pts->fY);
            needMove = false;
        }
        pts -= n;
        switch (v) {
            case kMove_Verb:
                if (needClose) {
                    this->close();
                    needClose = false;
                }
                needMove = true;
                pts += 1;   // so we see the point in "if (needMove)" above
                break;
            case kLine_Verb:
                this->lineTo(pts[0]);
                break;
            case kQuad_Verb:
                this->quadTo(pts[1], pts[0]);
                break;
            case kConic_Verb:
                this->conicTo(pts[1], pts[0], *--conicWeights);
                break;
            case kCubic_Verb:
                this->cubicTo(pts[2], pts[1], pts[0]);
                break;
            case kClose_Verb:
                needClose = true;
                break;
            default:
                SkDEBUGFAIL("unexpected verb");
        }
    }
    return *this;
}