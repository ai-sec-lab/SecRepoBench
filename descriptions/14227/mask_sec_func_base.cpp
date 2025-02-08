void SkGlyphRunListPainter::drawForBitmapDevice(
        const SkGlyphRunList& glyphRunList, const SkMatrix& deviceMatrix,
        const BitmapDevicePainter* bitmapDevice) {
    ScopedBuffers _ = this->ensureBuffers(glyphRunList);

    const SkPaint& runPaint = glyphRunList.paint();
    // The bitmap blitters can only draw lcd text to a N32 bitmap in srcOver. Otherwise,
    // convert the lcd text into A8 text. The props communicates this to the scaler.
    auto& props = (kN32_SkColorType == fColorType && runPaint.isSrcOver())
                  ? fDeviceProps
                  : fBitmapFallbackProps;

    SkPoint origin = glyphRunList.origin();
    for (auto& glyphRun : glyphRunList) {
        const SkFont& runFont = glyphRun.font();
        auto runSize = glyphRun.runSize();

        if (ShouldDrawAsPath(runPaint, runFont, deviceMatrix)) {
            SkMatrix::MakeTrans(origin.x(), origin.y()).mapPoints(
                    fPositions, glyphRun.positions().data(), runSize);
            // setup our std pathPaint, in hopes of getting hits in the cache
            SkPaint pathPaint(runPaint);
            SkFont  pathFont{runFont};
            SkScalar textScale = pathFont.setupForAsPaths(&pathPaint);

            // The sub-pixel position will always happen when transforming to the screen.
            pathFont.setSubpixel(false);

            SkAutoDescriptor ad;
            SkScalerContextEffects effects;
            SkScalerContext::CreateDescriptorAndEffectsUsingPaint(pathFont,
                                                                  pathPaint,
                                                                  props,
                                                                  fScalerContextFlags,
                                                                  SkMatrix::I(),
                                                                  &ad,
                                                                  &effects);

            SkScopedStrike strike =
                    fStrikeCache->findOrCreateScopedStrike(
                            *ad.getDesc(), effects,*pathFont.getTypefaceOrDefault());

            auto glyphPosSpan = strike->prepareForDrawing(
                    glyphRun.glyphsIDs().data(), fPositions, glyphRun.runSize(), 0, fGlyphPos);

            SkTDArray<SkPathPos> pathsAndPositions;
            pathsAndPositions.setReserve(glyphPosSpan.size());
            for (const SkGlyphPos& glyphPos : glyphPosSpan) {
                const SkGlyph& glyph = *glyphPos.glyph;
                SkPoint position = glyphPos.position;
                if (check_glyph_position(position)
                    && !glyph.isEmpty()
                    && glyph.fMaskFormat != SkMask::kARGB32_Format
                    && strike->decideCouldDrawFromPath(glyph))
                {
                    // Only draw a path if it exists, and this is not a color glyph.
                    pathsAndPositions.push_back(SkPathPos{glyph.path(), position});
                } else {
                    // TODO: this is here to have chrome layout tests pass. Remove this when
                    //  fallback for CPU works.
                    if (check_glyph_position(position)
                        && !glyph.isEmpty()
                        && strike->decideCouldDrawFromPath(glyph))
                    {
                        pathsAndPositions.push_back(SkPathPos{glyph.path(), position});
                    }
                }
            }

            // The paint we draw paths with must have the same anti-aliasing state as the runFont
            // allowing the paths to have the same edging as the glyph masks.
            pathPaint = runPaint;
            pathPaint.setAntiAlias(runFont.hasSomeAntiAliasing());

            bitmapDevice->paintPaths(
                    SkSpan<const SkPathPos>{pathsAndPositions.begin(), pathsAndPositions.size()},
                    textScale, pathPaint);
        } else {
            // <MASK>
            bitmapDevice->paintMasks(SkSpan<const SkMask>{masks.begin(), masks.size()}, runPaint);
        }
    }
}