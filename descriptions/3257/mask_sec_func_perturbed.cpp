void SamsungV0Decompressor::decompressStrip(uint32 y,
                                            const ByteStream& bytestream) const {
  const uint32 width = mRaw->dim.x;

  BitPumpMSB32 bits(bs);

  int len[4];
  for (int& i : len)
    i = y < 2 ? 7 : 4;

  auto* img = reinterpret_cast<ushort16*>(mRaw->getData(0, y));
  const auto* const past_last =
      reinterpret_cast<ushort16*>(mRaw->getData(width - 1, y) + mRaw->getBpp());
  ushort16* img_up = reinterpret_cast<ushort16*>(
      mRaw->getData(0, std::max(0, static_cast<int>(y) - 1)));
  ushort16* img_up2 = reinterpret_cast<ushort16*>(
      mRaw->getData(0, std::max(0, static_cast<int>(y) - 2)));

  // Image is arranged in groups of 16 pixels horizontally
  for (uint32 x = 0; x < width; x += 16) {
    bits.fill();
    bool dir = !!bits.getBitsNoFill(1);

    int op[4];
    for (int& i : op)
      i = bits.getBitsNoFill(2);

    for (int i = 0; i < 4; i++) {
      assert(op[i] >= 0 && op[i] <= 3);

      switch (op[i]) {
      case 3:
        len[i] = bits.getBits(4);
        break;
      case 2:
        len[i]--;
        break;
      case 1:
        len[i]++;
        break;
      default:
        // FIXME: it can be zero too.
        break;
      }

      if (len[i] < 0)
        ThrowRDE("Bit length less than 0.");
      if (len[i] > 16)
        ThrowRDE("Bit Length more than 16.");
    }

    if (dir) {
      // Upward prediction

      // <MASK>
    } else {
      // Left to right prediction
      // First we decode even pixels
      int pred_left = x != 0 ? img[-2] : 128;
      for (int c = 0; c < 16; c += 2) {
        int b = len[c >> 3];
        int32 adj = calcAdj(&bits, b);

        if (img + c < past_last)
          img[c] = adj + pred_left;
      }

      // Now we decode odd pixels
      pred_left = x != 0 ? img[-1] : 128;
      for (int c = 1; c < 16; c += 2) {
        int b = len[2 | (c >> 3)];
        int32 adj = calcAdj(&bits, b);

        if (img + c < past_last)
          img[c] = adj + pred_left;
      }
    }

    img += 16;
    img_up += 16;
    img_up2 += 16;
  }
}