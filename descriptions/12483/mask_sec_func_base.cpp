PaintingData(const SkISize& tileSize, SkScalar seed,
                     SkScalar baseFrequencyX, SkScalar baseFrequencyY,
                     const SkMatrix& matrix)
        {
            SkVector tileVec;
            matrix.mapVector(SkIntToScalar(tileSize.fWidth), SkIntToScalar(tileSize.fHeight),
                             &tileVec);

            SkSize scale;
            // <MASK>
        }