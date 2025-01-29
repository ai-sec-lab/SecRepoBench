{
    TiffEntry *ifdoffset = mRootIFD->getEntryRecursive(KODAK_IFD);
    if (!ifdoffset)
      ThrowRDE("Couldn't find the Kodak IFD offset");

    NORangesSet<Buffer> ifds;

    assert(ifdoffset != nullptr);
    TiffRootIFD kodakifd(nullptr, &ifds, ifdoffset->getRootIfdData(),
                         ifdoffset->getU32());

    TiffEntry *linearization = kodakifd.getEntryRecursive(KODAK_LINEARIZATION);
    if (!linearization || linearization->count != 1024 ||
        linearization->type != TIFF_SHORT)
      ThrowRDE("Couldn't find the linearization table");

    assert(linearization != nullptr);
    auto linearizationTable = linearization->getU16Array(1024);

    RawImageCurveGuard curveHandler(&mRaw, linearizationTable, uncorrectedRawValues);

    // FIXME: dcraw does all sorts of crazy things besides this to fetch
    //        WB from what appear to be presets and calculate it in weird ways
    //        The only file I have only uses this method, if anybody careas look
    //        in dcraw.c parse_kodak_ifd() for all that weirdness
    TiffEntry* blob = kodakifd.getEntryRecursive(static_cast<TiffTag>(0x03fd));
    if (blob && blob->count == 72) {
      mRaw->metadata.wbCoeffs[0] = 2048.0F / blob->getU16(20);
      mRaw->metadata.wbCoeffs[1] = 2048.0F / blob->getU16(21);
      mRaw->metadata.wbCoeffs[2] = 2048.0F / blob->getU16(22);
    }

    decodeKodak65000(&input, width, height);
  } else
    ThrowRDE("Unsupported compression %d", compression);