RawImage DcrDecoder::decodeRawInternal() {
  SimpleTiffDecoder::prepareForRawDecoding();

  ByteStream input(mFile, off);

  int compression = raw->getEntry(COMPRESSION)->getU32();
  if (65000 == compression) // <MASK>

  return mRaw;
}