for (auto y = 0; y < ROI.getHeight(); y += rowPitch) {
      for (auto x = 0; x < ROI.getWidth(); x += colPitch) {
        for (auto p = 0U; p < planes; ++p) {
          T& pixel =
              img(ROI.getTop() + y, firstPlane + (ROI.getLeft() + x) * cpp + p);
          pixel = op(x, y, pixel);
        }
      }
    }