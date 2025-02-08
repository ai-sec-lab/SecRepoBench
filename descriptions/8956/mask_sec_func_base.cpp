bool Parse<SkPoint>(const Value& v, SkPoint* pt) {
    if (!v.is<ObjectValue>())
        return false;
    const auto& ov = v.as<ObjectValue>();

    const auto& jvx = ov["x"];
    const auto& jvy = ov["y"];

    // Some BM versions seem to store x/y as single-element arrays.
    // <MASK>
}