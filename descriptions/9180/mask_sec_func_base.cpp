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
    uint32 off = sony_offset->getU32();

    assert(sony_length != nullptr);
    // <MASK>
  }
}