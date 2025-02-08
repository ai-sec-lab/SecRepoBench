bool Parse<SkPoint>(const Value& v, SkPoint* pt) {
    if (!v.is<ObjectValue>())
        return false;
    const auto& ov = v.as<ObjectValue>();

    const auto& jvx = ov["x"];
    const auto& jvy = ov["y"];

    // Some BM versions seem to store x/y as single-element arrays.
    // TODO: We should be able to check size == 1 below, or just delegate to Parse<SkScalar>,
    //       but that change introduces diffs.  Investigate.
    const ArrayValue* jvxa = jvx;
    const ArrayValue* jvya = jvy;
    return Parse<SkScalar>(jvxa && jvxa->size() > 0 ? (*jvxa)[0] : jvx, &pt->fX)
        && Parse<SkScalar>(jvya && jvya->size() > 0 ? (*jvya)[0] : jvy, &pt->fY);
}