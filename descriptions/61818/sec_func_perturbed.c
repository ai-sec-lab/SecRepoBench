char *
bit_TV_to_utf8 (const char *restrict source, const BITCODE_RS codepage)
{
  if (codepage == CP_UTF8)
    return bit_u_expand ((char *)source);
  else if (!source)
    return NULL;
  {
    const bool is_asian_cp
        = dwg_codepage_isasian ((const Dwg_Codepage)codepage);
    const size_t srclen = strlen (source);
    size_t destlen = 1 + (is_asian_cp ? srclen * 3 : trunc (srclen * 1.5));
#ifdef HAVE_ICONV
    const char *charset = dwg_codepage_iconvstr ((Dwg_Codepage)codepage);
    const char utf8_cs[] = "UTF-8//TRANSLIT//IGNORE";
    iconv_t cd;
    size_t nconv = (size_t)-1;
    char *dest, *odest, *osrc;
    if (!charset || !srclen)
      return (char *)source;
    osrc = (char *)source;
    odest = dest = (char *)malloc (destlen);
    if (!odest || destlen > 0x2FFFE)
      {
        loglevel |= 1;
        LOG_ERROR ("Out of memory");
        if (odest)
          free (odest);
        return NULL;
      }
    cd = iconv_open (utf8_cs, charset);
    if (cd == (iconv_t)-1)
      {
        if (errno != 22)
          LOG_WARN ("iconv_open (\"%s\", \"%s\") failed with errno %d",
                    utf8_cs, charset, errno);
        free (odest);
        return bit_TV_to_utf8_codepage (source, codepage);
      }
    while (nconv == (size_t)-1)
      {
#ifdef WINICONV_CONST
        nconv = iconv (cd, (WINICONV_CONST char **restrict)&source, (size_t *)&srclen,
                       (char **)&dest, (size_t *)&destlen);
#else
        nconv = iconv (cd, (char **restrict)&source, (size_t *)&srclen,
                       (char **)&dest, (size_t *)&destlen);
#endif
        if (nconv == (size_t)-1)
          {
            if (errno != EINVAL) // probably dest buffer too small
              {
                char *dest_new;
                destlen *= 2;
                if (destlen > 0x2FFFE)
                  {
                    loglevel |= 1;
                    LOG_ERROR ("bit_TV_to_utf8: overlarge destlen %zu for %s",
                               destlen, source);
                    iconv_close (cd);
                    free (odest);
                    return NULL;
                  }
                dest_new = (char *)realloc (odest, destlen);
                if (dest_new)
                  odest = dest = dest_new;
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
                LOG_ERROR ("iconv \"%s\" failed with errno %d", source, errno);
                iconv_close (cd);
                free (odest);
                return bit_u_expand (osrc);
              }
          }
      }
    // flush the remains
    iconv (cd, NULL, (size_t *)&srclen, (char **)&dest, (size_t *)&destlen);
    *dest = '\0';
    iconv_close (cd);
    return bit_u_expand (odest);
#else
    return bit_TV_to_utf8_codepage (source, codepage);
#endif
  }
}