try {
    NikonDecompressor::decompress(
        &mRaw, ByteStream(mFile, offsets->getU32(), counts->getU32()),
        meta->getData(), mRaw->dim, bitPerPixel, uncorrectedRawValues);
  } catch (IOException &e) {
    mRaw->setError(e.what());
    // Let's ignore it, it may have delivered somewhat useful data.
  }

  return mRaw;