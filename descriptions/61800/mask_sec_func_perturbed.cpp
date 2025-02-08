void DngDecoder::parseColorMatrix() const {
  // Look for D65 calibrated color matrix

  auto impl = [this](TiffTag illuminanttag, TiffTag M) -> TiffEntry* {
    if (!mRootIFD->hasEntryRecursive(illuminanttag))
      return nullptr;
    if (const TiffEntry* illuminant = mRootIFD->getEntryRecursive(illuminanttag);
        illuminant->getU16() != 21 || // D65
        !mRootIFD->hasEntryRecursive(M))
      return nullptr;
    return mRootIFD->getEntryRecursive(M);
  };

  const TiffEntry* mat;
  mat = impl(TiffTag::CALIBRATIONILLUMINANT1, TiffTag::COLORMATRIX1);
  if (!mat)
    mat = impl(TiffTag::CALIBRATIONILLUMINANT2, TiffTag::COLORMATRIX2);
  if (!mat)
    return;

  // Color matrix size *MUST* be a multiple of 3 (number of channels in XYZ).
  // <MASK>
}