void Cr2Decompressor::decodeN_X_Y()
{
  auto ht = getHuffmanTables<N_COMP>();
  auto pred = getInitialPredictors<N_COMP>();
  auto predNext = reinterpret_cast<ushort16*>(mRaw->getDataUncropped(0, 0));

  BitPumpJPEG bitStream(input);

  uint32 pixelPitch = mRaw->pitch / 2; // Pitch in pixel
  if (frame.cps != 3 && frame.w * frame.cps > 2 * frame.h) {
    // Fix Canon double height issue where Canon doubled the width and halfed
    // the height (e.g. with 5Ds), ask Canon. frame.w needs to stay as is here
    // because the number of pixels after which the predictor gets updated is
    // still the doubled width.
    // see: FIX_CANON_HALF_HEIGHT_DOUBLE_WIDTH
    frame.h *= 2;
  }

  if (X_S_F == 2 && Y_S_F == 1)
  {
    // fix the inconsistent slice width in sRaw mode, ask Canon.
    for (auto& sliceWidth : slicesWidths)
      sliceWidth = sliceWidth * 3 / 2;
  }

  for (const auto& slicesWidth : slicesWidths) {
    if (slicesWidth > mRaw->dim.x)
      ThrowRDE("Slice is longer than image's height, which is unsupported.");
  }

  // <MASK>
}