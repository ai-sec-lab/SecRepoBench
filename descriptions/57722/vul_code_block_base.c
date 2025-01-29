cd = iconv_open ("UTF-8", charset);
    if (cd == (iconv_t) -1)
      {
        loglevel |= 1;
        LOG_ERROR ("iconv_open (\"UTF-8\", \"%s\") failed with errno %d",
                   charset, errno);
        return NULL;
      }
    osrc = (char *)src;
    odest = dest = (char*)malloc (destlen);
    while (nconv == (size_t)-1)
      {
        nconv = iconv (cd, (char **restrict)&src, (size_t *)&srclen,
                       (char **)&dest, (size_t *)&destlen);
        if (nconv == (size_t)-1)
          {
            if (errno != EINVAL) // probably dest buffer too small
              {
                char *dest_new;
                destlen *= 2;
                dest_new = (char*)realloc (odest, destlen);
                if (dest_new)
                  odest = dest = dest_new;
              }
            else
              {
                loglevel |= 1;
                LOG_ERROR ("iconv \"%s\" failed with errno %d", src, errno);
                free (odest);
                iconv_close (cd);
                return bit_u_expand (osrc);
              }
          }
      }