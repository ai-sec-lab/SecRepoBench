          p=PushLongPixel(quantum_info->endian,p,&pixel);
          if (x++ < (ssize_t) (number_pixels-1))
            {
              SetPixelGray(img,ScaleAnyToQuantum((pixel >> 2) & 0x3ff,
                range),q);
              q+=GetPixelChannels(img);
            }
          if (x++ < (ssize_t) number_pixels)
            {
              SetPixelGray(img,ScaleAnyToQuantum((pixel >> 12) & 0x3ff,
                range),q);
              q+=GetPixelChannels(img);
            }
          break;