try {
    l.decode({width});
  } catch (IOException& e) {
    mRaw->setError(e.what());
  }

  // deal with D2000 GrayResponseCurve
  TiffEntry* curve = mRootIFD->getEntryRecursive(static_cast<TiffTag>(0x123));
  if (curve && curve->type == TIFF_SHORT && curve->count == 4096) {
    auto table = curve->getU16Array(curve->count);
    RawImageCurveGuard curveHandler(&mRaw, table, uncorrectedRawValues);

    // Apply table
    if (!uncorrectedRawValues)
      mRaw->sixteenBitLookup();
  }

  return mRaw;