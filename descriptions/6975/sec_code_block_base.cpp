for (const auto& slicesWidth : slicesWidths) {
    if (slicesWidth > mRaw->dim.x)
      ThrowRDE("Slice is longer than image's height, which is unsupported.");
    if (slicesWidth % xStepSize != 0) {
      ThrowRDE("Slice width (%u) should be multiple of pixel group size (%u)",
               slicesWidth, xStepSize);
    }
  }

  if (frame.h * std::accumulate(slicesWidths.begin(), slicesWidths.end(), 0) <
      mRaw->getCpp() * mRaw->dim.area())
    ThrowRDE("Incorrrect slice height / slice widths! Less than image size.");

  unsigned processedPixels = 0;