if (!codepage)
    return (char *)src;
  //  UTF8 encode
  while (i < destlen && (char*)tmp < &src[srclen] && (c = *tmp))
    {
      wchar_t wc;
      tmp++;
      if (is_asian_cp) // has exceptions even below 0x80
        {
          if (dwg_codepage_is_twobyte ((Dwg_Codepage)codepage, c))
            c = c << 8 | *tmp++;
          wc = dwg_codepage_uwc ((Dwg_Codepage)codepage, c);
          c = wc;
          // printf("wc: %u\n", (unsigned)wc);
          if (c < 0x80) // stayed below
            str[i++] = c & 0xFF;
        }
      else if (c < 0x80)
        str[i++] = c & 0xFF;
      else if ((wc = dwg_codepage_uc ((Dwg_Codepage)codepage, c & 0xFF)))
        {
          c = wc;
          // printf("wc: %u\n", (unsigned)wc);
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
        { /* windows ucs-2 has no D800-DC00 surrogate pairs. go straight up
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
    }