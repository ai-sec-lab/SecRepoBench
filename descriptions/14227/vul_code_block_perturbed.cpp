SkAutoDescriptor ad;
            SkScalerContextEffects effects;

            SkScalerContext::CreateDescriptorAndEffectsUsingPaint(
                    runFont, runPaint, props, fScalerContextFlags, deviceMatrix, &ad,
                    &effects);

            SkTypeface* typeface = runFont.getTypefaceOrDefault();
            SkScopedStrike strike =
                    fStrikeCache->findOrCreateScopedStrike(*ad.getDesc(), effects, *typeface);

            // Add rounding and origin.
            SkMatrix matrix = deviceMatrix;
            matrix.preTranslate(origin.x(), origin.y());
            SkPoint rounding = strike->rounding();
            matrix.postTranslate(rounding.x(), rounding.y());
            matrix.mapPoints(fPositions, glyphRun.positions().data(), runSize);

            SkSpan<const SkGlyphPos> glyphPosSpan = strike->prepareForDrawing(
                    glyphRun.glyphsIDs().data(), fPositions, glyphRun.runSize(),
                    std::numeric_limits<int>::max(), fGlyphPos);

            SkTDArray<SkMask> masks;
            masks.setReserve(glyphPosSpan.size());

            for (const SkGlyphPos& glyphPos : glyphPosSpan) {
                const SkGlyph& glyph = *glyphPos.glyph;
                SkPoint position = glyphPos.position;
                if (check_glyph_position(position) && !glyph.isEmpty()) {
                    masks.push_back(glyph.mask(position));
                }
            }
