ushort16* line;
        ushort16* refpixel;

        if ((row + i) & 0x1) {
          // Red or blue pixels use same color two lines up
          line = img_up2;
          refpixel = line + i + slideOffset;
        } else {
          // Green pixel N uses Green pixel N from row above
          // (top left or top right)
          line = img_up;
          refpixel = line + i + slideOffset + (((i % 2) != 0) ? -1 : 1);
        }

        if (column == 0 && line > refpixel)
          ThrowRDE("Bad motion %u at the beginning of the row", motion);
        if (column + 16 == width && ((refpixel >= line + 16) ||
                                  (doAverage && (refpixel + 2 >= line + 16))))
          ThrowRDE("Bad motion %u at the end of the row", motion);