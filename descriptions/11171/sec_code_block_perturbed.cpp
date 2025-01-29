SkScalar width = rectangleBounds.width();
        SkScalar height = rectangleBounds.height();
        if (!SkScalarIsFinite(width) || !SkScalarIsFinite(height)) {
            return false;
        }

        // We want vertexCount grid cells, roughly distributed to match the bounds ratio
        SkScalar hCount = SkScalarSqrt(sk_ieee_float_divide(vertexCount*width, height));
        if (!SkScalarIsFinite(hCount)) {
            return false;
        }
        fHCount = SkTMax(SkTMin(SkScalarRoundToInt(hCount), vertexCount), 1);
        fVCount = vertexCount/fHCount;
        fGridConversion.set(sk_ieee_float_divide(fHCount - 0.001f, width),
                            sk_ieee_float_divide(fVCount - 0.001f, height));