if (x + 16 >= width)
        ThrowRDE("Upward prediction for the last block of pixels. Raw corrupt");

      // First we decode even pixels
      for (int c = 0; c < 16; c += 2) {
        int b = len[c >> 3];
        int32 adj = calcAdj(&bits, b);

        img[c] = adj + img_up[c];
      }

      // Now we decode odd pixels
      // Why on earth upward prediction only looks up 1 line above
      // is beyond me, it will hurt compression a deal.
      for (int c = 1; c < 16; c += 2) {
        int b = len[2 | (c >> 3)];
        int32 adj = calcAdj(&bits, b);

        img[c] = adj + img_up2[c];
      }