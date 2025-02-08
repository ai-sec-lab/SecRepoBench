while (nconv == (size_t)-1)
      {
#ifdef WINICONV_CONST
        nconv = iconv (cd, (WINICONV_CONST char **restrict)&src, (size_t *)&srclen,
                       (char **)&dest, (size_t *)&destinationlength);
#else
        nconv = iconv (cd, (char **restrict)&src, (size_t *)&srclen,
                       (char **)&dest, (size_t *)&destinationlength);
#endif
        if (nconv == (size_t)-1)
          {
            if (errno != EINVAL) // probably dest buffer too small
              {
                char *dest_new;
                destinationlength *= 2;
                if (destinationlength > 0x2FFFE)
                  {
                    loglevel |= 1;
                    LOG_ERROR ("bit_TV_to_utf8: overlarge destlen %zu for %s",
                               destinationlength, src);
                    iconv_close (cd);
                    free (odest);
                    return NULL;
                  }
                dest_new = (char *)realloc (odest, destinationlength);
                if (dest_new)
                  {
                    odest = dest = dest_new;
                    odestlen = destinationlength;
                  }
                else
                  {
                    loglevel |= 1;
                    LOG_ERROR ("Out of memory");
                    iconv_close (cd);
                    //free (odest);
                    return NULL;
                  }
              }
            else
              {
                loglevel |= 1;
                LOG_ERROR ("iconv \"%s\" failed with errno %d", src, errno);
                iconv_close (cd);
                free (odest);
                return bit_u_expand (osrc);
              }
          }
      }
    // flush the remains
    iconv (cd, NULL, (size_t *)&srclen, (char **)&dest, (size_t *)&destinationlength);
    if (errno == 0 && dest >= odest && dest <= &odest[odestlen])
      {
        *dest = '\0';
        iconv_close (cd);
        return bit_u_expand (odest);
      }
    else
      {
        iconv_close (cd);
        free (odest);
        return bit_TV_to_utf8_codepage (src, codepage);
      }