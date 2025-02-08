void RawDecoder::decodeUncompressed(const TiffIFD *rawIFD, BitOrder order) {
  TiffEntry *offsets = rawIFD->getEntry(STRIPOFFSETS);
  TiffEntry *counts = rawIFD->getEntry(STRIPBYTECOUNTS);
  uint32 yPerSlice = rawIFD->getEntry(ROWSPERSTRIP)->getU32();
  uint32 width = rawIFD->getEntry(IMAGEWIDTH)->getU32();
  uint32 height = rawIFD->getEntry(IMAGELENGTH)->getU32();
  uint32 bitPerPixel = rawIFD->getEntry(BITSPERSAMPLE)->getU32();

  if (width == 0 || height == 0 || width > 5632 || height > 3720)
    ThrowRDE("Unexpected image dimensions found: (%u; %u)", width, height);

  mRaw->dim = iPoint2D(width, height);

  if (counts->count != offsets->count) {
    ThrowRDE("Byte count number does not match strip size: "
             "count:%u, stips:%u ",
             counts->count, offsets->count);
  }

  if (yPerSlice == 0 || yPerSlice > static_cast<uint32>(mRaw->dim.y) ||
      roundUpDivision(mRaw->dim.y, yPerSlice) != counts->count) {
    ThrowRDE("Invalid y per slice %u or strip count %u (height = %u)",
             yPerSlice, counts->count, mRaw->dim.y);
  }

  switch (bitPerPixel) {
  case 12:
  case 14:
    break;
  default:
    ThrowRDE("Unexpected bits per pixel: %u.", bitPerPixel);
  };

  vector<RawSlice> slices;
  slices.reserve(counts->count);
  uint32 offsetY = 0;

  for (uint32 s = 0; s < counts->count; s++) {
    RawSlice slice;
    slice.offset = offsets->getU32(s);
    slice.count = counts->getU32(s);

    if (slice.count < 1)
      ThrowRDE("Slice %u is empty", s);

    if (offsetY + yPerSlice > height)
      slice.h = height - offsetY;
    else
      slice.h = yPerSlice;

    offsetY += yPerSlice;

    if (!mFile->isValid(slice.offset, slice.count))
      ThrowRDE("Slice offset/count invalid");

    slices.push_back(slice);
  }

  if (slices.empty())
    ThrowRDE("No valid slices found. File probably truncated.");

  assert(height <= offsetY);
  assert(slices.size() == counts->count);

  mRaw->createData();

  // Default white level is (2 ** BitsPerSample) - 1
  mRaw->whitePoint = (1UL << bitPerPixel) - 1UL;

  offsetY = 0;
  for (const RawSlice& slice : slices) {
    UncompressedDecompressor u(*mFile, slice.offset, slice.count, mRaw);
    iPoint2D size(width, slice.h);
    iPoint2D pos(0, offsetY);
    bitPerPixel = static_cast<int>(
        static_cast<uint64>(static_cast<uint64>(slice.count) * 8U) /
        (slice.h * width));
    const auto inputPitch = width * bitPerPixel / 8;
    if (!inputPitch)
      ThrowRDE("Bad input pitch. Can not decode anything.");

    u.readUncompressedRaw(size, pos, inputPitch, bitPerPixel, order);

    offsetY += slice.h;
  }
}