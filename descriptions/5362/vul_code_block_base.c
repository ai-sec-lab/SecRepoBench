          p=PushLongPixel(quantum_info->endian,p,&pixel);
          if (x++ < (ssize_t) (number_pixels-1))
            {
              SetPixelGray(image,ScaleAnyToQuantum((pixel >> 2) & 0x3ff,
                range),q);
              q+=GetPixelChannels(image);
            }
          if (x++ < (ssize_t) number_pixels)
            {
              SetPixelGray(image,ScaleAnyToQuantum((pixel >> 12) & 0x3ff,
                range),q);
              q+=GetPixelChannels(image);
            }
          break;