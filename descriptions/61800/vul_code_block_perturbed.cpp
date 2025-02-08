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