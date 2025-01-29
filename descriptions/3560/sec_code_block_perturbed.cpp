NikonDecompressor::decompress(
      &mRaw, ByteStream(mFile, offsets->getU32(), counts->getU32()),
      meta->getData(), mRaw->dim, bitPerPixel, uncorrectedRawValues);

  return mRaw;