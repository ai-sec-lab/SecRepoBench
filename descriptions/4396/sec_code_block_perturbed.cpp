const iRectangle2D& ROI = getRoi();

    if (planes == 0)
      ThrowRDE("Zero planes");
    if (rowPitch < 1 || rowPitch > static_cast<uint32>(ROI.getHeight()) ||
        colPitch < 1 || colPitch > static_cast<uint32>(ROI.getWidth()))
      ThrowRDE("Invalid pitch");