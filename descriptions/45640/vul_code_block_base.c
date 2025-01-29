if (map.red != (int *) NULL)
            for (i=0; i < (ssize_t) colors; i++)
            {
              index=map.red[i];
              if ((info.scale != (Quantum *) NULL) && (index >= 0) &&
                  (index <= (int) info.max_value))
                index=(int) info.scale[index];
              image->colormap[i].red=(MagickRealType) index;
            }
          if (map.green != (int *) NULL)
            for (i=0; i < (ssize_t) colors; i++)
            {
              index=map.green[i];
              if ((info.scale != (Quantum *) NULL) && (index >= 0) &&
                  (index <= (int) info.max_value))
                index=(int) info.scale[index];
              image->colormap[i].green=(MagickRealType) index;
            }
          if (map.blue != (int *) NULL)
            for (i=0; i < (ssize_t) colors; i++)
            {
              index=map.blue[i];
              if ((info.scale != (Quantum *) NULL) && (index >= 0) &&
                  (index <= (int) info.max_value))
                index=(int) info.scale[index];
              image->colormap[i].blue=(MagickRealType) index;
            }
          if (map.gray != (int *) NULL)
            for (i=0; i < (ssize_t) colors; i++)
            {
              index=map.gray[i];
              if ((info.scale != (Quantum *) NULL) && (index >= 0) &&
                  (index <= (int) info.max_value))
                index=(int) info.scale[index];
              image->colormap[i].red=(MagickRealType) index;
              image->colormap[i].green=(MagickRealType) index;
              image->colormap[i].blue=(MagickRealType) index;
            }