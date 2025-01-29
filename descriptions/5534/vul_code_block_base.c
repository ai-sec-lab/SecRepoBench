for (i = 0; i < 4; i++)
        {
          if ((x + i) < (ssize_t) image->rows && (y + j) < (ssize_t) image->columns)
            {
              code = (bits >> ((4*j+i)*2)) & 0x3;
              SetPixelRed(image,ScaleCharToQuantum(colors.r[code]),q);
              SetPixelGreen(image,ScaleCharToQuantum(colors.g[code]),q);
              SetPixelBlue(image,ScaleCharToQuantum(colors.b[code]),q);
              /*
                Extract alpha value: multiply 0..15 by 17 to get range 0..255
              */
              if (j < 2)
                alpha = 17U * (unsigned char) ((a0 >> (4*(4*j+i))) & 0xf);
              else
                alpha = 17U * (unsigned char) ((a1 >> (4*(4*(j-2)+i))) & 0xf);
              SetPixelAlpha(image,ScaleCharToQuantum((unsigned char) alpha),q);
              q+=GetPixelChannels(image);
            }
        }