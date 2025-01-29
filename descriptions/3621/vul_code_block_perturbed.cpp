try {
    if (offsets->count != 1 || hints.has("force_uncompressed"))
      decodeUncompressed(input, width, height, size);
    else
      decodeCompressed(&input, width, height);
  } catch (IOException &e) {
     mRaw->setError(e.what());
  }

  return mRaw;