// TODO: We should be able to check size == 1 below, or just delegate to Parse<SkScalar>,
    //       but that change introduces diffs.  Investigate.
    const ArrayValue* jvxa = jvx;
    const ArrayValue* jvya = jvy;
    return Parse<SkScalar>(jvxa && jvxa->size() > 0 ? (*jvxa)[0] : jvx, &point->fX)
        && Parse<SkScalar>(jvya && jvya->size() > 0 ? (*jvya)[0] : jvy, &point->fY);