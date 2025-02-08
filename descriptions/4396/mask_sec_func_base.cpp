explicit PixelOpcode(const RawImage& ri, ByteStream* bs) : ROIOpcode(ri, bs) {
    firstPlane = bs->getU32();
    planes = bs->getU32();
    rowPitch = bs->getU32();
    colPitch = bs->getU32();

    // <MASK>
  }