
        // We want vertexCount grid cells, roughly distributed to match the bounds ratio
        SkScalar hCount = SkScalarSqrt(sk_ieee_float_divide(vertexCount*bounds.width(),
                                                            bounds.height()));
        if (!SkScalarIsFinite(hCount)) {
            return false;
        }
        fHCount = SkTMax(SkTMin(SkScalarRoundToInt(hCount), vertexCount), 1);
        fVCount = vertexCount/fHCount;
        fGridConversion.set(sk_ieee_float_divide(fHCount - 0.001f, bounds.width()),
                            sk_ieee_float_divide(fVCount - 0.001f, bounds.height()));