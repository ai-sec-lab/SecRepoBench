RawImage NefDecoder::decodeRawInternal() {
  auto raw = mRootIFD->getIFDWithTag(CFAPATTERN);
  int compression = raw->getEntry(COMPRESSION)->getU32();

  TiffEntry *offsets = raw->getEntry(STRIPOFFSETS);
  TiffEntry *counts = raw->getEntry(STRIPBYTECOUNTS);

  if (mRootIFD->getEntryRecursive(MODEL)->getString() == "NIKON D100 ") { /**Sigh**/
    if (!mFile->isValid(offsets->getU32()))
      ThrowRDE("Image data outside of file.");
    if (!D100IsCompressed(offsets->getU32())) {
      DecodeD100Uncompressed();
      return mRaw;
    }
  }

  if (compression == 1 || (hints.has("force_uncompressed")) ||
      NEFIsUncompressed(raw)) {
    DecodeUncompressed();
    return mRaw;
  }

  if (NEFIsUncompressedRGB(raw)) {
    DecodeSNefUncompressed();
    return mRaw;
  }

  if (offsets->count != 1) {
    ThrowRDE("Multiple Strips found: %u", offsets->count);
  }
  if (counts->count != offsets->count) {
    ThrowRDE(
        "Byte count number does not match strip size: count:%u, strips:%u ",
        counts->count, offsets->count);
  }
  if (!mFile->isValid(offsets->getU32(), counts->getU32()))
    ThrowRDE("Invalid strip byte count. File probably truncated.");

  if (34713 != compression)
    ThrowRDE("Unsupported compression");

  uint32 width = raw->getEntry(IMAGEWIDTH)->getU32();
  uint32 height = raw->getEntry(IMAGELENGTH)->getU32();
  uint32 bitPerPixel = raw->getEntry(BITSPERSAMPLE)->getU32();

  if (width == 0 || height == 0 || width % 2 != 0 || width > 8288 ||
      height > 5520)
    ThrowRDE("Unexpected image dimensions found: (%u; %u)", width, height);

  switch (bitPerPixel) {
  case 12:
  case 14:
    break;
  default:
    ThrowRDE("Invalid bpp found: %u", bitPerPixel);
  }

  mRaw->dim = iPoint2D(width, height);
  mRaw->createData();

  raw = mRootIFD->getIFDWithTag(static_cast<TiffTag>(0x8c));

  TiffEntry *meta;
  if (raw->hasEntry(static_cast<TiffTag>(0x96))) {
    meta = raw->getEntry(static_cast<TiffTag>(0x96));
  } else {
    meta = raw->getEntry(static_cast<TiffTag>(0x8c)); // Fall back
  }

  NikonDecompressor::decompress(
      &mRaw, ByteStream(mFile, offsets->getU32(), counts->getU32()),
      meta->getData(), mRaw->dim, bitPerPixel, uncorrectedRawValues);

  return mRaw;
}