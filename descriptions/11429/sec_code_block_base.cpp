for (; count > 0; --count) {
      if (iPixel >= nPixels)
        ThrowRDE("Buffer overflow");
      data[iPixel] = dequantize(pixelValue);
      ++iPixel;
    }