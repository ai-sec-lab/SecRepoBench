auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(
                                        runFont, runPaint, props,
                                        fScalerContextFlags, deviceMatrix);

            // Add rounding and origin.
            SkMatrix matrix = deviceMatrix;
            matrix.preTranslate(origin.x(), origin.y());
            SkPoint rounding = cache->rounding();
            matrix.postTranslate(rounding.x(), rounding.y());
            matrix.mapPoints(fPositions, glyphRun.positions().data(), runSize);

            SkTDArray<SkMask> masks;
            masks.setReserve(runSize);
            const SkPoint* positionCursor = fPositions;
            for (auto glyphID : glyphRun.glyphsIDs()) {
                auto position = *positionCursor++;
                if (check_glyph_position(position)) {
                    const SkGlyph& glyph = cache->getGlyphMetrics(glyphID, position);
                    const void* image;
                    if (!glyph.isEmpty() && (image = cache->findImage(glyph))) {
                        masks.push_back(create_mask(glyph, position, image));
                    }
                }
            }