ushort16* refpixel;

        if ((row + i) & 0x1) {
          // Red or blue pixels use same color two lines up
          refpixel = img_up2 + i + slideOffset;

          if (column == 0 && img_up2 > refpixel)
            ThrowRDE("Bad motion %u at the beginning of the row", motion);
          if (column + 16 == width &&
              ((refpixel >= img_up2 + 16) ||
               (doAverage && (refpixel + 2 >= img_up2 + 16))))
            ThrowRDE("Bad motion %u at the end of the row", motion);
        } else {
          // Green pixel N uses Green pixel N from row above
          // (top left or top right)
          refpixel = img_up + i + slideOffset + (((i % 2) != 0) ? -1 : 1);

          if (column == 0 && img_up > refpixel)
            ThrowRDE("Bad motion %u at the beginning of the row", motion);
        }
