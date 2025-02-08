float circleR = circle.width() / 2.0f;
    if (circleR < SK_ScalarNearlyZero) {
        return nullptr;
    }

    auto threadSafeCache = rContext->priv().threadSafeCache();