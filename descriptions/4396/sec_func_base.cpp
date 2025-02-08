explicit PixelOpcode(const RawImage& ri, ByteStream* bs) : ROIOpcode(ri, bs) {
    firstPlane = bs->getU32();
    planes = bs->getU32();
    rowPitch = bs->getU32();
    colPitch = bs->getU32();

    const iRectangle2D& ROI = getRoi();

    if (planes == 0)
      ThrowRDE("Zero planes");
    if (rowPitch < 1 || rowPitch > static_cast<uint32>(ROI.getHeight()) ||
        colPitch < 1 || colPitch > static_cast<uint32>(ROI.getWidth()))
      ThrowRDE("Invalid pitch");
  }