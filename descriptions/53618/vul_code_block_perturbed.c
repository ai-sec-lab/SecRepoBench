  if (mRaw->getDataType() != RawImageType::UINT16)
    ThrowRDE("Unexpected data type");

  if (mRaw->getCpp() != 1 || mRaw->getBpp() != sizeof(uint16_t))
    ThrowRDE("Unexpected cpp: %u", mRaw->getCpp());

  if (!((std::make_tuple(3, 2, 2) == format) ||
        (std::make_tuple(3, 2, 1) == format) ||
        (std::make_tuple(2, 1, 1) == format) ||
        (std::make_tuple(4, 1, 1) == format)))
    ThrowRDE("Unknown format <%i,%i,%i>", std::get<0>(format),
             std::get<1>(format), std::get<2>(format));

  const Dsc dsc(format);

  dim = mRaw->dim;
  if (!dim.hasPositiveArea() || dim.x % dsc.groupSize != 0)
    ThrowRDE("Unexpected image dimension multiplicity");
  dim.x /= dsc.groupSize;

  if (!frame.hasPositiveArea() || frame.x % dsc.X_S_F != 0 ||
      frame.y % dsc.Y_S_F != 0)
    ThrowRDE("Unexpected LJpeg frame dimension multiplicity");
  frame.x /= dsc.X_S_F;
  frame.y /= dsc.Y_S_F;

  if (mRaw->dim.x > 19440 || mRaw->dim.y > 5920) {
    ThrowRDE("Unexpected image dimensions found: (%u; %u)", mRaw->dim.x,
             mRaw->dim.y);
  }

  for (auto sliceId = 0; sliceId < slicing.numSlices; sliceId++) {
    const auto sliceWidth = slicing.widthOfSlice(sliceId);
    if (sliceWidth <= 0)
      ThrowRDE("Bad slice width: %i", sliceWidth);
  }

  if (dsc.subSampled == mRaw->isCFA)
    ThrowRDE("Cannot decode subsampled image to CFA data or vice versa");

  if (static_cast<int>(rec.size()) != dsc.N_COMP)
    ThrowRDE("HT/Initial predictor count does not match component count");

  for (const auto& recip : rec) {
    if (!recip.ht.isFullDecode())
      ThrowRDE("Huffman table is not of a full decoding variety");
  }

  for (auto* width : {&slicing.sliceWidth, &slicing.lastSliceWidth}) {
    if (*width % dsc.sliceColStep != 0) {
      ThrowRDE("Slice width (%u) should be multiple of pixel group size (%u)",
               *width, dsc.sliceColStep);
    }
    *width /= dsc.sliceColStep;
  }

  const iRectangle2D fullImage({0, 0}, dim);
  for (iRectangle2D output : getOutputTiles()) {
    if (output.getLeft() == dim.x)
      break;
    if (!output.isThisInside(fullImage))
      ThrowRDE("Output tile not inside of the image");
  }