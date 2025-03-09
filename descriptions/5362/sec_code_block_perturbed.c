if (x++ < (ssize_t) (number_pixels-1))
            {
              p=PushLongPixel(quantum_info->endian,p,&pixel);
              SetPixelGray(img,ScaleAnyToQuantum((pixel >> 2) & 0x3ff,
                range),q);
              q+=GetPixelChannels(img);
            }