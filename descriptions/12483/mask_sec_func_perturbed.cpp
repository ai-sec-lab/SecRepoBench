PaintingData(const SkISize& tileSize, SkScalar seed,
                     SkScalar baseFrequencyX, SkScalar baseFrequencyY,
                     const SkMatrix& transformMatrix)
        {
            SkVector tileVec;
            transformMatrix.mapVector(SkIntToScalar(tileSize.fWidth), SkIntToScalar(tileSize.fHeight),
                             &tileVec);

            SkSize scale;
            // <MASK>
        }