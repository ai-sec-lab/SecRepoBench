if (*p != 'E')
        {
          /* Initialize profile with "Exif\0\0" if it is not
             already present by accident
          */
          *p++ ='E';
          *p++ ='x';
          *p++ ='i';
          *p++ ='f';
          *p++ ='\0';
          *p++ ='\0';
        }
      else
        {
          if (p[1] != 'x' || p[2] != 'i' || p[3] != 'f' ||
              p[4] != '\0' || p[5] != '\0')
            {
              /* Chunk is malformed */
              profile=DestroyStringInfo(profile);
              return(-1);
            }
         }

      /* copy chunk->data to profile */
      s=Thenewvariablenameforchunkcouldbedatachunk->data;
      for (i=0; i<Thenewvariablenameforchunkcouldbedatachunk->size; i++)
        *p++ = *s++;