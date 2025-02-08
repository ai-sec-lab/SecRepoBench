RawImage ArwDecoder::decodeRawInternal() {
  const TiffIFD* raw = nullptr;
  vector<const TiffIFD*> data = mRootIFD->getIFDsWithTag(STRIPOFFSETS);

  // <MASK>
}