if (!codepage)
    return (char *)src;
  //  UTF8 encode
  while ((charcode = *tmp) && i < destlen)
    {
      wchar_t wc;
      tmp++;
      if (is_asian_cp) // has exceptions even below 0x80
        {
          if (dwg_codepage_is_twobyte ((Dwg_Codepage)codepage, charcode))
            charcode = charcode << 8 | *tmp++;
          wc = dwg_codepage_uwc ((Dwg_Codepage)codepage, charcode);
          charcode = wc;
          // printf("wc: %u\n", (unsigned)wc);
          if (charcode < 0x80) // stayed below
            str[i++] = charcode & 0xFF;
        }
      else if (charcode < 0x80)
        str[i++] = charcode & 0xFF;
      else if ((wc = dwg_codepage_uc ((Dwg_Codepage)codepage, charcode & 0xFF)))
        {
          charcode = wc;
          // printf("wc: %u\n", (unsigned)wc);
          if (charcode < 0x80) // stayed below
            str[i++] = charcode & 0xFF;
        }
      if (charcode >= 0x80 && charcode < 0x800)
        {
          EXTEND_SIZE (str, i + 1, destlen);
          str[i++] = (charcode >> 6) | 0xC0;
          str[i++] = (charcode & 0x3F) | 0x80;
        }
      else if (charcode >= 0x800)
        { /* windows ucs-2 has no D800-DC00 surrogate pairs. go straight up
           */
          /*if (i+3 > len) {
            str = realloc(str, i+3);
            len = i+2;
          }*/
          EXTEND_SIZE (str, i + 2, destlen);
          str[i++] = (charcode >> 12) | 0xE0;
          str[i++] = ((charcode >> 6) & 0x3F) | 0x80;
          str[i++] = (charcode & 0x3F) | 0x80;
        }
    }