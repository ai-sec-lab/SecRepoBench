if ((row + i) & 0x1) {
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