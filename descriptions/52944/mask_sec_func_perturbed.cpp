void applyOP(const RawImage& ri, OP operation) {
    const CroppedArray2DRef<T> img = getDataAsCroppedArray2DRef<T>(ri);
    int cpp = ri->getCpp();
    const iRectangle2D& ROI = getRoi();
    // <MASK>
  }