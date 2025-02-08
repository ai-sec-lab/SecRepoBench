explicit PixelOpcode(const RawImage& ri, ByteStream* byteStream) : ROIOpcode(ri, byteStream) {
    firstPlane = byteStream->getU32();
    planes = byteStream->getU32();
    rowPitch = byteStream->getU32();
    colPitch = byteStream->getU32();

    const iRectangle2D& ROI = getRoi();

    if (planes == 0)
      ThrowRDE("Zero planes");
    if (rowPitch < 1 || rowPitch > static_cast<uint32>(ROI.getHeight()) ||
        colPitch < 1 || colPitch > static_cast<uint32>(ROI.getWidth()))
      ThrowRDE("Invalid pitch");
  }