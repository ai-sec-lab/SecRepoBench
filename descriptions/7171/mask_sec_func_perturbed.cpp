void Cr2Decompressor::decodeN_X_Y()
{
  // To understand the CR2 slice handling and sampling factor behavior, see
  // https://github.com/lclevy/libcraw2/blob/master/docs/cr2_lossless.pdf?raw=true

  // inner loop decodes one group of pixels at a time
  //  * for <N,1,1>: N  = N*1*1 (full raw)
  //  * for <3,2,1>: 6  = 3*2*1
  //  * for <3,2,2>: 12 = 3*2*2
  // and advances x by N_COMP*X_S_F and y by Y_S_F
  constexpr int xPixelGroupSize = N_COMP * X_S_F;
  constexpr int yStepSize = Y_S_F;

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
    for (auto* width : {&slicing.sliceWidth, &slicing.lastSliceWidth})
      *width = (*width) * 3 / 2;
  }

  for (const auto& width : {slicing.sliceWidth, slicing.lastSliceWidth}) {
    if (width > mRaw->dim.x)
      ThrowRDE("Slice is longer than image's height, which is unsupported.");
    if (width % xPixelGroupSize != 0) {
      ThrowRDE("Slice width (%u) should be multiple of pixel group size (%u)",
               width, xPixelGroupSize);
    }
  }

  if (iPoint2D::area_type(frame.h) * slicing.totalWidth() <
      mRaw->getCpp() * mRaw->dim.area())
    ThrowRDE("Incorrrect slice height / slice widths! Less than image size.");

  unsigned processedPixels = 0;
  unsigned processedLineSlices = 0;
  for (auto sliceId = 0; sliceId < slicing.numSlices; sliceId++) {
    const unsigned sliceWidth = slicing.widthOfSlice(sliceId);

    assert(frame.h % yStepSize == 0);
    for (unsigned y = 0; y < frame.h; y += yStepSize) {
      // Fix for Canon 80D mraw format.
      // In that format, `frame` is 4032x3402, while `mRaw` is 4536x3024.
      // Consequently, the slices in `frame` wrap around plus there are few
      // 'extra' sliced lines because sum(slicesW) * sliceH > mRaw->dim.area()
      // Those would overflow, hence the break.
      // see FIX_CANON_FRAME_VS_IMAGE_SIZE_MISMATCH
      unsigned destY = processedLineSlices % mRaw->dim.y;
      // <MASK>
    }
  }
}