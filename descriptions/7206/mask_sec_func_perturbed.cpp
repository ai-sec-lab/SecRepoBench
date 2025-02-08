void ArwDecoder::GetWB() {
  // Set the whitebalance for all the modern ARW formats (everything after A100)
  if (mRootIFD->hasEntryRecursive(DNGPRIVATEDATA)) {
    NORangesSet<Buffer> ifds_undecoded;

    TiffEntry *priv = mRootIFD->getEntryRecursive(DNGPRIVATEDATA);
    TiffRootIFD makerNoteIFD(nullptr, &ifds_undecoded, priv->getRootIfdData(),
                             priv->getU32());

    TiffEntry *sony_offset = makerNoteIFD.getEntryRecursive(SONY_OFFSET);
    TiffEntry *sony_length = makerNoteIFD.getEntryRecursive(SONY_LENGTH);
    TiffEntry *sony_key = makerNoteIFD.getEntryRecursive(SONY_KEY);
    if(!sony_offset || !sony_length || !sony_key || sony_key->count != 4)
      ThrowRDE("couldn't find the correct metadata for WB decoding");

    assert(sony_offset != nullptr);
    uint32 offset = sony_offset->getU32();

    assert(sony_length != nullptr);
    uint32 len = sony_length->getU32();

    assert(sony_key != nullptr);
    uint32 key = getU32LE(sony_key->getData(4));

    // "Decrypt" IFD
    const auto& ifd_crypt = priv->getRootIfdData();
    // <MASK>

    DataBuffer dbIDD(decIFD, priv->getRootIfdData().getByteOrder());
    TiffRootIFD encryptedIFD(nullptr, &ifds_decoded, dbIDD, offset);

    if (encryptedIFD.hasEntry(SONYGRBGLEVELS)){
      TiffEntry *wb = encryptedIFD.getEntry(SONYGRBGLEVELS);
      if (wb->count != 4)
        ThrowRDE("WB has %d entries instead of 4", wb->count);
      mRaw->metadata.wbCoeffs[0] = wb->getFloat(1);
      mRaw->metadata.wbCoeffs[1] = wb->getFloat(0);
      mRaw->metadata.wbCoeffs[2] = wb->getFloat(2);
    } else if (encryptedIFD.hasEntry(SONYRGGBLEVELS)){
      TiffEntry *wb = encryptedIFD.getEntry(SONYRGGBLEVELS);
      if (wb->count != 4)
        ThrowRDE("WB has %d entries instead of 4", wb->count);
      mRaw->metadata.wbCoeffs[0] = wb->getFloat(0);
      mRaw->metadata.wbCoeffs[1] = wb->getFloat(1);
      mRaw->metadata.wbCoeffs[2] = wb->getFloat(3);
    }
  }
}