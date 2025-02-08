void applyOP(const RawImage& ri, OP operation) {
    const CroppedArray2DRef<T> img = getDataAsCroppedArray2DRef<T>(ri);
    int cpp = ri->getCpp();
    const iRectangle2D& ROI = getRoi();
    const iPoint2D numAffected(roundUpDivision(getRoi().dim.x, colPitch),
                               roundUpDivision(getRoi().dim.y, rowPitch));
    for (int y = 0; y < numAffected.y; ++y) {
      for (int x = 0; x < numAffected.x; ++x) {
        for (auto p = 0U; p < planes; ++p) {
          T& pixel = img(ROI.getTop() + rowPitch * y,
                         firstPlane + (ROI.getLeft() + colPitch * x) * cpp + p);
          pixel = operation(x, y, pixel);
        }
      }
    }
  }