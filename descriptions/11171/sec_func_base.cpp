bool init(const SkRect& bounds, int vertexCount) {
        fBounds = bounds;
        fNumVerts = 0;
        SkScalar width = bounds.width();
        SkScalar height = bounds.height();
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
        if (!fGridConversion.isFinite()) {
            return false;
        }

        fGrid.setCount(fHCount*fVCount);
        for (int i = 0; i < fGrid.count(); ++i) {
            fGrid[i].reset();
        }

        return true;
    }