float circleR = circle.width() / 2.0f;
    if (!sk_float_isfinite(circleR) || circleR < SK_ScalarNearlyZero) {
        return nullptr;
    }

    auto threadSafeCache = rContext->priv().threadSafeCache();