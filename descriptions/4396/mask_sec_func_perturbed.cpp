explicit PixelOpcode(const RawImage& ri, ByteStream* byteStream) : ROIOpcode(ri, byteStream) {
    firstPlane = byteStream->getU32();
    planes = byteStream->getU32();
    rowPitch = byteStream->getU32();
    colPitch = byteStream->getU32();

    // <MASK>
  }