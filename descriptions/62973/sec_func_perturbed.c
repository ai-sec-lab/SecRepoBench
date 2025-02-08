char *
bit_TV_to_utf8 (const char *restrict inputstring, const BITCODE_RS codepage)
{
  if (codepage == CP_UTF8)
    return bit_u_expand ((char *)inputstring);
  else if (!inputstring)
    return NULL;
  {
    const bool is_asian_cp
        = dwg_codepage_isasian ((const Dwg_Codepage)codepage);
    const size_t srclen = strlen (inputstring);
    size_t destlen = 1 + (is_asian_cp ? srclen * 3 : trunc (srclen * 2));
#ifdef HAVE_ICONV
    const char *charset = dwg_codepage_iconvstr ((Dwg_Codepage)codepage);
    const char utf8_cs[] = "UTF-8//TRANSLIT//IGNORE";
    iconv_t cd;
    size_t nconv = (size_t)-1;
    char *dest, *odest, *osrc;
    size_t odestlen = destlen;
    if (!charset || !srclen)
      return (char *)inputstring;
    osrc = (char *)inputstring;
    odest = dest = (char *)calloc (odestlen, 1);
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
        return bit_TV_to_utf8_codepage (inputstring, codepage);
      }
    while (nconv == (size_t)-1)
      {
#  ifdef WINICONV_CONST
        nconv = iconv (cd, (WINICONV_CONST char **restrict)&inputstring,
                       (size_t *)&srclen, (char **)&dest, (size_t *)&destlen);
#  else
        nconv = iconv (cd, (char **restrict)&inputstring, (size_t *)&srclen,
                       (char **)&dest, (size_t *)&destlen);
#  endif
        if (nconv == (size_t)-1)
          {
            if (errno != EINVAL) // probably dest buffer too small
              {
                char *dest_new;
                destlen *= 2;
                if (destlen > 0x2FFFE)
                  {
                    loglevel |= 1;
                    LOG_ERROR ("bit_TV_to_utf8: overlarge destlen %" PRIuSIZE
                               " for %s",
                               destlen, inputstring);
                    iconv_close (cd);
                    free (odest);
                    return NULL;
                  }
                dest_new = (char *)realloc (odest, destlen);
                if (dest_new)
                  {
                    odest = dest = dest_new;
                    odestlen = destlen;
                    dest_new[destlen - 1] = '\0';
                  }
                else
                  {
                    loglevel |= 1;
                    LOG_ERROR ("Out of memory");
                    iconv_close (cd);
                    // free (odest);
                    return NULL;
                  }
              }
            else
              {
                loglevel |= 1;
                LOG_ERROR ("iconv \"%s\" failed with errno %d", inputstring, errno);
                iconv_close (cd);
                free (odest);
                return bit_u_expand (osrc);
              }
          }
      }
    // flush the remains
    iconv (cd, NULL, NULL, (char **)&dest, (size_t *)&destlen);
    if (errno == 0 && destlen <= 0x2FFFE && (uintptr_t)dest >= (uintptr_t)odest
        && (uintptr_t)dest <= (uintptr_t)odest + odestlen)
      {
        //*dest = '\0';
        iconv_close (cd);
        return bit_u_expand (odest);
      }
    else
      {
        iconv_close (cd);
        free (odest);
        return bit_TV_to_utf8_codepage (inputstring, codepage);
      }
#else
    return bit_TV_to_utf8_codepage (inputstring, codepage);
#endif
  }
}