char *
bit_TV_to_utf8 (const char *restrict src, const BITCODE_RS codepage)
{
  if (codepage == CP_UTF8)
    return bit_u_expand ((char *)src);
  else if (!src)
    return NULL;
  {
    const bool is_asian_cp
        = dwg_codepage_isasian ((const Dwg_Codepage)codepage);
    const size_t srclen = strlen (src);
    size_t destinationlength = 1 + (is_asian_cp ? srclen * 3 : trunc (srclen * 1.5));
#ifdef HAVE_ICONV
    const char *charset = dwg_codepage_iconvstr ((Dwg_Codepage)codepage);
    const char utf8_cs[] = "UTF-8//TRANSLIT//IGNORE";
    iconv_t cd;
    size_t nconv = (size_t)-1;
    char *dest, *odest, *osrc;
    size_t odestlen = destinationlength;
    if (!charset || !srclen)
      return (char *)src;
    osrc = (char *)src;
    odest = dest = (char *)malloc (odestlen);
    if (!odest || destinationlength > 0x2FFFE)
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
        return bit_TV_to_utf8_codepage (src, codepage);
      }
    // <MASK>
#else
    return bit_TV_to_utf8_codepage (src, codepage);
#endif
  }
}