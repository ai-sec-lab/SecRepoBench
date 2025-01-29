switch (bitPerPixel) {
  case 12:
  case 14:
    break;
  default:
    ThrowRDE("Unexpected bits per pixel: %u.", bitPerPixel);
  };

  vector<RawSlice> slices;
  uint32 offY = 0;

  for (uint32 s = 0; s < nslices; s++) {
    RawSlice slice;
    slice.offset = offsets->getU32(s);
    slice.count = counts->getU32(s);
    if (offY + yPerSlice > height)
      slice.h = height - offY;
    else
      slice.h = yPerSlice;

    offY += yPerSlice;

    if (mFile->isValid(slice.offset, slice.count)) // Only decode if size is valid
      slices.push_back(slice);
  }

  if (slices.empty())
    ThrowRDE("No valid slices found. File probably truncated.");

  mRaw->dim = iPoint2D(width, offY);
  mRaw->createData();

  // Default white level is (2 ** BitsPerSample) - 1
  mRaw->whitePoint = (1UL << bitPerPixel) - 1UL;

  offY = 0;
  for (uint32 i = 0; i < slices.size(); i++) {
    RawSlice slice = slices[i];
    UncompressedDecompressor u(*mFile, slice.offset, slice.count, mRaw);
    iPoint2D size(width, slice.h);
    iPoint2D pos(0, offY);
    bitPerPixel = static_cast<int>(
        static_cast<uint64>(static_cast<uint64>(slice.count) * 8U) /
        (slice.h * width));
    const auto inputPitch = width * bitPerPixel / 8;
    if (!inputPitch)
      ThrowRDE("Bad input pitch. Can not decode anything.");
    try {
      u.readUncompressedRaw(size, pos, inputPitch, bitPerPixel, order);
    } catch (RawDecoderException &e) {
      if (i>0)
        mRaw->setError(e.what());
      else
        throw;
    } catch (IOException &e) {
      if (i>0)
        mRaw->setError(e.what());
      else {
        ThrowRDE("IO error occurred in first slice, unable to decode more. "
                 "Error is: %s",
                 e.what());
      }
    }
    offY += slice.h;
  }