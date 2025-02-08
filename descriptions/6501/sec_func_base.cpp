size_t SkPath::readFromMemory_EQ4(const void* storage, size_t length) {
    SkRBuffer buffer(storage, length);
    uint32_t packed;
    if (!buffer.readU32(&packed)) {
        return 0;
    }

    SkASSERT(extract_version(packed) == 4);

    switch (extract_serializationtype(packed)) {
        case SerializationType::kRRect:
            return this->readAsRRect(storage, length);
        case SerializationType::kGeneral:
            break;  // fall through
        default:
            return 0;
    }

    int32_t pts, cnx, vbs;
    if (!buffer.readS32(&pts) || !buffer.readS32(&cnx) || !buffer.readS32(&vbs)) {
        return 0;
    }

    const SkPoint* points = buffer.skipCount<SkPoint>(pts);
    const SkScalar* conics = buffer.skipCount<SkScalar>(cnx);
    const uint8_t* verbs = buffer.skipCount<uint8_t>(vbs);
    buffer.skipToAlign4();
    if (!buffer.isValid()) {
        return 0;
    }
    SkASSERT(buffer.pos() <= length);

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

    *this = std::move(tmp);
    return buffer.pos();
}