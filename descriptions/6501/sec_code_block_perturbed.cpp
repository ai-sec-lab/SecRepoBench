#define CHECK_POINTS_CONICS(p, c)       \
    do {                                \
        if (p && ((pts -= p) < 0)) {    \
            return 0;                   \
        }                               \
        if (c && ((cnx -= c) < 0)) {    \
            return 0;                   \
        }                               \
    } while (0)

    SkPath tmp;
    tmp.setFillType(extract_filltype(packed));
    tmp.incReserve(pts);
    for (int i = vbs - 1; i >= 0; --i) {
        switch (verbs[i]) {
            case kMove_Verb:
                CHECK_POINTS_CONICS(1, 0);
                tmp.moveTo(*points++);
                break;
            case kLine_Verb:
                CHECK_POINTS_CONICS(1, 0);
                tmp.lineTo(*points++);
                break;
            case kQuad_Verb:
                CHECK_POINTS_CONICS(2, 0);
                tmp.quadTo(points[0], points[1]);
                points += 2;
                break;
            case kConic_Verb:
                CHECK_POINTS_CONICS(2, 1);
                tmp.conicTo(points[0], points[1], *conics++);
                points += 2;
                break;
            case kCubic_Verb:
                CHECK_POINTS_CONICS(3, 0);
                tmp.cubicTo(points[0], points[1], points[2]);
                points += 3;
                break;
            case kClose_Verb:
                tmp.close();
                break;
            default:
                return 0;   // bad verb
        }
    }
#undef CHECK_POINTS_CONICS
    if (pts || cnx) {
        return 0;   // leftover points and/or conics
    }