for (uint32 col = 0; col < width; col += 16) {
    if (!(optflags & OptFlags::QP) && !(col & 63)) {
      static constexpr int32 scalevals[] = {0, -2, 2};
      uint32 i = pump.getBits(2);
      scale = i < 3 ? scale + scalevals[i] : pump.getBits(12);
    }

    // First we figure out which reference pixels mode we're in
    if (optflags & OptFlags::MV)
      motion = pump.getBits(1) ? 3 : 7;
    else if (!pump.getBits(1))
      motion = pump.getBits(3);

    if ((line == 0 || line == 1) && (motion != 7))
      ThrowRDE("At start of image and motion isn't 7. File corrupted?");

    if (motion == 7) {
      // The base case, just set all pixels to the previous ones on the same
      // line If we're at the left edge we just start at the initial value
      for (uint32 i = 0; i < 16; i++)
        img[i] = (col == 0) ? initVal : *(img + i - 2);
    } else {
      // The complex case, we now need to actually lookup one or two lines
      // above
      if (line < 2)
        ThrowRDE(
            "Got a previous line lookup on first two lines. File corrupted?");

      static constexpr int32 motionOffset[7] = {-4, -2, -2, 0, 0, 2, 4};
      static constexpr int32 motionDoAverage[7] = {0, 0, 1, 0, 1, 0, 0};

      int32 slideOffset = motionOffset[motion];
      int32 doAverage = motionDoAverage[motion];

      for (uint32 i = 0; i < 16; i++) {
        ushort16* refpixel;

        if ((line + i) & 0x1) {
          // Red or blue pixels use same color two lines up
          refpixel = img_up2 + i + slideOffset;

          if (col == 0 && img_up2 > refpixel)
            ThrowRDE("Bad motion %u at the beginning of the row", motion);
        } else {
          // Green pixel N uses Green pixel N from row above
          // (top left or top right)
          refpixel = img_up + i + slideOffset + (((i % 2) != 0) ? -1 : 1);

          if (col == 0 && img_up > refpixel)
            ThrowRDE("Bad motion %u at the beginning of the row", motion);
        }

        // In some cases we use as reference interpolation of this pixel and
        // the next
        if (doAverage)
          img[i] = (*refpixel + *(refpixel + 2) + 1) >> 1;
        else
          img[i] = *refpixel;
      }
    }

    // Figure out how many difference bits we have to read for each pixel
    uint32 diffBits[4] = {0};
    if (optflags & OptFlags::SKIP || !pump.getBits(1)) {
      uint32 flags[4];
      for (unsigned int& flag : flags)
        flag = pump.getBits(2);

      for (uint32 i = 0; i < 4; i++) {
        // The color is 0-Green 1-Blue 2-Red
        uint32 colornum = (line % 2 != 0) ? i >> 1 : ((i >> 1) + 2) % 3;

        assert(flags[i] <= 3);
        switch (flags[i]) {
        case 0:
          diffBits[i] = diffBitsMode[colornum][0];
          break;
        case 1:
          diffBits[i] = diffBitsMode[colornum][0] + 1;
          break;
        case 2:
          diffBits[i] = diffBitsMode[colornum][0] - 1;
          break;
        case 3:
          diffBits[i] = pump.getBits(4);
          break;
        default:
          __builtin_unreachable();
        }

        diffBitsMode[colornum][0] = diffBitsMode[colornum][1];
        diffBitsMode[colornum][1] = diffBits[i];

        if (diffBits[i] > bitDepth + 1)
          ThrowRDE("Too many difference bits. File corrupted?");
      }
    }

    // Actually read the differences and write them to the pixels
    for (uint32 i = 0; i < 16; i++) {
      uint32 len = diffBits[i >> 2];
      int32 diff = pump.getBits(len);

      // If the first bit is 1 we need to turn this into a negative number
      if (len != 0 && diff >> (len - 1))
        diff -= (1 << len);

      ushort16* value = nullptr;
      // Apply the diff to pixels 0 2 4 6 8 10 12 14 1 3 5 7 9 11 13 15
      if (line % 2)
        value = &img[((i & 0x7) << 1) + 1 - (i >> 3)];
      else
        value = &img[((i & 0x7) << 1) + (i >> 3)];

      diff = diff * (scale * 2 + 1) + scale;
      *value = clampBits(static_cast<int>(*value) + diff, bits);
    }

    img += 16;
    img_up += 16;
    img_up2 += 16;
  }

  data.skipBytes(pump.getBufferPosition());