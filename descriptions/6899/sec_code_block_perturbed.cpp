for (auto slicesWidth : slicesWidths) {
    if (slicesWidth <= 0)
      ThrowRDE("Bad slice width: %i", slicesWidth);
  }

  AbstractLJpegDecompressor::decode();