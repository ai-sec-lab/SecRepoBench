explicit FixBadPixelsList(const RawImage& ri, ByteStream* byteStream) {
    const iRectangle2D fullImage(0, 0, ri->getUncroppedDim().x - 1,
                                 ri->getUncroppedDim().y - 1);

    byteStream->getU32(); // Skip phase - we don't care
    auto badPointCount = byteStream->getU32();
    auto badRectCount = byteStream->getU32();

    byteStream->check(2 * 4 * badPointCount + 4 * 4 * badRectCount);

    // Read points
    badPixels.reserve(badPixels.size() + badPointCount);
    for (auto i = 0U; i < badPointCount; ++i) {
      auto y = byteStream->getU32();
      auto x = byteStream->getU32();

      const iPoint2D badPoint(x, y);
      if (!fullImage.isPointInside(badPoint))
        ThrowRDE("Bad point not inside image.");

      badPixels.emplace_back(y << 16 | x);
    }

    // Read rects
    for (auto i = 0U; i < badRectCount; ++i) {
      auto top = byteStream->getU32();
      auto left = byteStream->getU32();
      auto bottom = byteStream->getU32();
      auto right = byteStream->getU32();

      const iRectangle2D badRect(left, top, right - left, bottom - top);
      if (!badRect.isThisInside(fullImage))
        ThrowRDE("Bad rectangle not inside image.");

      auto area = (1 + bottom - top) * (1 + right - left);
      badPixels.reserve(badPixels.size() + area);
      for (auto y = top; y <= bottom; ++y) {
        for (auto x = left; x <= right; ++x) {
          badPixels.emplace_back(y << 16 | x);
        }
      }
    }
  }