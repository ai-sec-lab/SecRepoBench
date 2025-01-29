if (chunk->size > 6)
        {
          /* Skip first 6 bytes if "Exif\0\0" is
             already present by accident
          */
          if (s[0] == 'E' && s[1] == 'x'  && s[2] == 'i' &&
              s[3] == 'f' && s[4] == '\0' && s[5] == '\0')
          {
            s+=6;
            i=6;
            SetStringInfoLength(profile,chunk->size);
          }
        }

      /* copy chunk->data to profile */
      for (; i<chunk->size; i++)
        *p++ = *s++;

      error_info=(PNGErrorInfo *) png_get_error_ptr(ping);
      (void) SetImageProfile(image,"exif",profile,
        error_info->exception);

      profile=DestroyStringInfo(profile);

      return(1);