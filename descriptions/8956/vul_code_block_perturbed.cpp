return Parse<SkScalar>(jvx.is<ArrayValue>() ? jvx.as<ArrayValue>()[0] : jvx, &point->fX)
        && Parse<SkScalar>(jvy.is<ArrayValue>() ? jvy.as<ArrayValue>()[0] : jvy, &point->fY);