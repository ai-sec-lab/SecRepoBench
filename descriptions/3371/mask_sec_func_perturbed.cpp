void RawDecoder::decodeUncompressed(const TiffIFD *rawIFD, BitOrder order) {
  uint32 nslices = rawIFD->getEntry(STRIPOFFSETS)->count;
  TiffEntry *offsets = rawIFD->getEntry(STRIPOFFSETS);
  TiffEntry *counts = rawIFD->getEntry(STRIPBYTECOUNTS);
  uint32 rowsPerSlice = rawIFD->getEntry(ROWSPERSTRIP)->getU32();
  uint32 width = rawIFD->getEntry(IMAGEWIDTH)->getU32();
  uint32 height = rawIFD->getEntry(IMAGELENGTH)->getU32();
  uint32 bitPerPixel = rawIFD->getEntry(BITSPERSAMPLE)->getU32();

  if (width == 0 || height == 0 || width > 5632 || height > 3720)
    ThrowRDE("Unexpected image dimensions found: (%u; %u)", width, height);

  // <MASK>
}