SkPath& SkPath::reverseAddPath(const SkPath& srcPath) {
    // <MASK>

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