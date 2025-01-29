if (redmap != (int *) NULL)
          for (i=0; i < (ssize_t) colors; i++)
          {
            index=redmap[i];
            if ((info.scale != (Quantum *) NULL) && (index >= 0) &&
                (index <= (int) info.max_value))
              index=(int) info.scale[index];
            image->colormap[i].red=(MagickRealType) index;
          }
        if (greenmap != (int *) NULL)
          for (i=0; i < (ssize_t) colors; i++)
          {
            index=greenmap[i];
            if ((info.scale != (Quantum *) NULL) && (index >= 0) &&
                (index <= (int) info.max_value))
              index=(int) info.scale[index];
            image->colormap[i].green=(MagickRealType) index;
          }
        if (bluemap != (int *) NULL)
          for (i=0; i < (ssize_t) colors; i++)
          {
            index=bluemap[i];
            if ((info.scale != (Quantum *) NULL) && (index >= 0) &&
                (index <= (int) info.max_value))
              index=(int) info.scale[index];
            image->colormap[i].blue=(MagickRealType) index;
          }
        if (graymap != (int *) NULL)
          for (i=0; i < (ssize_t) colors; i++)
          {
            index=graymap[i];
            if ((info.scale != (Quantum *) NULL) && (index >= 0) &&
                (index <= (int) info.max_value))
              index=(int) info.scale[index];
            image->colormap[i].red=(MagickRealType) index;
            image->colormap[i].green=(MagickRealType) index;
            image->colormap[i].blue=(MagickRealType) index;
          }