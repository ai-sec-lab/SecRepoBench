void DngDecoder::parseColorMatrix() const {
  // Look for D65 calibrated color matrix

  auto impl = [this](TiffTag I, TiffTag M) -> TiffEntry* {
    if (!mRootIFD->hasEntryRecursive(I))
      return nullptr;
    if (const TiffEntry* illuminant = mRootIFD->getEntryRecursive(I);
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
  if (mat->count % 3 != 0)
    return;

  const auto srat_vals = mat->getSRationalArray(mat->count);
  bool Success = true;
  mRaw->metadata.colorMatrix.reserve(mat->count);
  for (const auto& val : srat_vals) {
    Success &= val.den != 0;
    if (!Success)
      break;
    mRaw->metadata.colorMatrix.emplace_back(val);
  }
  if (!Success)
    mRaw->metadata.colorMatrix.clear();
}