char *
bit_TV_to_utf8 (const char *restrict src,
                const BITCODE_RS codepage)
{
  if (codepage == CP_UTF8)
    return bit_u_expand ((char *)src);
  {
    const bool is_asian_cp
      = dwg_codepage_isasian ((const Dwg_Codepage)codepage);
    const size_t srclen = strlen (src);
    size_t destlen = is_asian_cp ? srclen * 3 : trunc (srclen * 1.5);
#ifdef HAVE_ICONV
    const char *charset = dwg_codepage_iconvstr ((Dwg_Codepage)codepage);
    iconv_t cd;
    size_t nconv = (size_t)-1;
    char *dest, *odest, *osrc;
    if (!charset)
      return (char*)src;
    // <MASK>
    // flush the remains
    iconv (cd, NULL, (size_t *)&srclen, (char **)&dest, (size_t *)&destlen);
    *dest = '\0';
    iconv_close (cd);
    return bit_u_expand (odest);
#else
    size_t i = 0;
    char *str = calloc (1, destlen + 1);
    char *tmp = (char *)src;
    uint16_t c = 0;
    //printf("cp: %u\n", codepage);
    //printf("src: %s\n", src);
    //printf("destlen: %zu\n", destlen);
    // UTF8 encode
    while ((c = (0xFF & *tmp)) && i < destlen)
      {
        wchar_t wc;
        tmp++;
        //printf("c: %hu\n", c);
        //printf("i: %zu\n", i);
        //printf("str: %s\n", str);
        //if (is_asian_cp)
        //  c = (c << 16) + *tmp++;
        if (c < 0x80)
          str[i++] = c & 0xFF;
        else if ((wc = dwg_codepage_uc ((Dwg_Codepage)codepage, c & 0xFF)))
          {
            c = wc;
            //printf("wc: %u\n", (unsigned)wc);
            if (c < 0x80) // stayed below
              str[i++] = c & 0xFF;
          }
        if (c >= 0x80 && c < 0x800)
          {
            EXTEND_SIZE (str, i + 1, destlen);
            str[i++] = (c >> 6) | 0xC0;
            str[i++] = (c & 0x3F) | 0x80;
          }
        else if (c >= 0x800)
          {  /* windows ucs-2 has no D800-DC00 surrogate pairs. go straight up
              */
            /*if (i+3 > len) {
              str = realloc(str, i+3);
              len = i+2;
            }*/
            EXTEND_SIZE (str, i + 2, destlen);
            str[i++] = (c >> 12) | 0xE0;
            str[i++] = ((c >> 6) & 0x3F) | 0x80;
            str[i++] = (c & 0x3F) | 0x80;
          }
        /*
        else if (c < 0x110000)
          {
            EXTEND_SIZE(str, i + 3, len);
            str[i++] = (c >> 18) | 0xF0;
            str[i++] = ((c >> 12) & 0x3F) | 0x80;
            str[i++] = ((c >> 6) & 0x3F) | 0x80;
            str[i++] = (c & 0x3F) | 0x80;
          }
        else
          HANDLER (OUTPUT, "ERROR: overlarge unicode codepoint U+%0X", c);
       */
      }
    //printf("=> str: %s, i: %zu\n", str, i);
    EXTEND_SIZE (str, i + 1, destlen);
    str[i] = '\0';
    return bit_u_expand (str);
#endif
  }
}