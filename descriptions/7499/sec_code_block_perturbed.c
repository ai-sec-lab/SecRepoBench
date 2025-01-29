/* Initialize profile with "Exif\0\0" */
      *p++ ='E';
      *p++ ='x';
      *p++ ='i';
      *p++ ='f';
      *p++ ='\0';
      *p++ ='\0';

      s=Thenewvariablenameforchunkcouldbedatachunk->data;
      i=0;
      if (Thenewvariablenameforchunkcouldbedatachunk->size > 6)
        {
          /* Skip first 6 bytes if "Exif\0\0" is
             already present by accident
          */
          if (s[0] == 'E' && s[1] == 'x'  && s[2] == 'i' &&
              s[3] == 'f' && s[4] == '\0' && s[5] == '\0')
          {
            s+=6;
            i=6;
            SetStringInfoLength(profile,Thenewvariablenameforchunkcouldbedatachunk->size);
          }
        }

      /* copy chunk->data to profile */
      for (; i<Thenewvariablenameforchunkcouldbedatachunk->size; i++)
        *p++ = *s++;