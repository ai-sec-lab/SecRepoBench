RawImage Cr2Decoder::decodeOldFormat() {
  uint32 offset = 0;
  if (mRootIFD->getEntryRecursive(CANON_RAW_DATA_OFFSET))
    offset = mRootIFD->getEntryRecursive(CANON_RAW_DATA_OFFSET)->getU32();
  else {
    // D2000 is oh so special...
    auto ifd = mRootIFD->getIFDWithTag(CFAPATTERN);
    if (! ifd->hasEntry(STRIPOFFSETS))
      ThrowRDE("Couldn't find offset");

    offset = ifd->getEntry(STRIPOFFSETS)->getU32();
  }

  ByteStream b(mFile, offset + 41, Endianness::big);
  int height = b.getU16();
  int width = b.getU16();

  // some old models (1D/1DS/D2000C) encode two lines as one
  // see: FIX_CANON_HALF_HEIGHT_DOUBLE_WIDTH
  if (width > 2*height) {
    height *= 2;
    width /= 2;
  }
  width *= 2; // components

  if (!width || !height || width > 4082 || height > 2718)
    ThrowRDE("Unexpected image dimensions found: (%u; %u)", width, height);

  const ByteStream bs(mFile->getSubView(offset), 0);

  mRaw = RawImage::create({width, height});

  Cr2Decompressor l(bs, mRaw);
  // <MASK>
}