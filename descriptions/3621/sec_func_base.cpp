RawImage OrfDecoder::decodeRawInternal() {
  auto raw = mRootIFD->getIFDWithTag(STRIPOFFSETS);

  int compression = raw->getEntry(COMPRESSION)->getU32();
  if (1 != compression)
    ThrowRDE("Unsupported compression");

  TiffEntry *offsets = raw->getEntry(STRIPOFFSETS);
  TiffEntry *counts = raw->getEntry(STRIPBYTECOUNTS);

  if (counts->count != offsets->count) {
    ThrowRDE(
        "Byte count number does not match strip size: count:%u, strips:%u ",
        counts->count, offsets->count);
  }

  //TODO: this code assumes that all strips are layed out directly after another without padding and in order
  uint32 off = raw->getEntry(STRIPOFFSETS)->getU32();
  uint32 size = 0;
  for (uint32 i=0; i < counts->count; i++)
    size += counts->getU32(i);

  if (!mFile->isValid(off, size))
    ThrowRDE("Truncated file");

  uint32 width = raw->getEntry(IMAGEWIDTH)->getU32();
  uint32 height = raw->getEntry(IMAGELENGTH)->getU32();

  if (!width || !height || width % 2 != 0 || width > 9280 || height > 6932)
    ThrowRDE("Unexpected image dimensions found: (%u; %u)", width, height);

  mRaw->dim = iPoint2D(width, height);
  mRaw->createData();

  ByteStream input(offsets->getRootIfdData());
  input.setPosition(off);

  if (offsets->count != 1 || hints.has("force_uncompressed"))
    decodeUncompressed(input, width, height, size);
  else
    decodeCompressed(&input, width, height);

  return mRaw;
}