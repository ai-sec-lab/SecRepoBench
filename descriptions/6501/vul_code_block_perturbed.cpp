    SkPath tmp;
    tmp.setFillType(extract_filltype(packed));
    tmp.incReserve(pts);
    for (int i = vbs - 1; i >= 0; --i) {
        switch (verbs[i]) {
            case kMove_Verb:  tmp.moveTo(*points++); break;
            case kLine_Verb:  tmp.lineTo(*points++); break;
            case kQuad_Verb:  tmp.quadTo(points[0], points[1]); points += 2; break;
            case kConic_Verb: tmp.conicTo(points[0], points[1], *conics++); points += 2; break;
            case kCubic_Verb: tmp.cubicTo(points[0], points[1], points[2]); points += 3; break;
            case kClose_Verb: tmp.close(); break;
            default:
                return 0;   // bad verb
        }
    }