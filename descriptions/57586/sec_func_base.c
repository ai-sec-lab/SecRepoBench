ATTRIBUTE_MALLOC
BITCODE_TU
bit_utf8_to_TU (char *restrict str, const unsigned cquoted)
{
  BITCODE_TU wstr;
  size_t i = 0;
  size_t len = strlen (str);
  unsigned char c;

  if (len > 0xFFFE)
    {
      loglevel = 1;
      LOG_WARN ("Overlong string truncated (len=%lu)",
                (unsigned long)len);
      len = UINT16_MAX - 1;
    }
  wstr = (BITCODE_TU)calloc (2, len + 1);
  if (!wstr)
    {
      loglevel = 1;
      LOG_ERROR ("Out of memory")
      return NULL;
    }
  while (len > 0 && (c = *str++))
    {
      len--;
      if (c < 128)
        {
          if (cquoted && c == '\\' && len > 1 &&
              // skip \" to " and \\ to \.
              (*str == '"' || *str == '\\' || *str == 'r'
               || *str == 'n'))
            {
              c = *str++;
              if (c == 'r')
                wstr[i++] = '\r';
              else if (c == 'n')
                wstr[i++] = '\n';
              // else skip \ and " from \\ and \"
              else
                wstr[i++] = c;
            }
          else
            wstr[i++] = c;
        }
      else if ((c & 0xe0) == 0xc0)
        {
          /* ignore invalid utf8 for now */
          if (len >= 1)
            {
              wstr[i++] = ((c & 0x1f) << 6) | (str[1] & 0x3f);
              len--;
              str++;
            }
        }
      else if ((c & 0xf0) == 0xe0)
        {
          /* ignore invalid utf8? */
          if (len >= 2
              && ((unsigned char)str[1] < 0x80 || (unsigned char)str[1] > 0xBF
                  || (unsigned char)str[2] < 0x80
                  || (unsigned char)str[2] > 0xBF))
            {
              LOG_WARN ("utf-8: BAD_CONTINUATION_BYTE %s", str);
            }
          else if (len >= 1 && c == 0xe0 && (unsigned char)str[1] < 0xa0)
            {
              LOG_WARN ("utf-8: NON_SHORTEST %s", str);
            }
          else if (len >= 2)
            {
              wstr[i++] = ((c & 0x0f) << 12) | ((str[1] & 0x3f) << 6)
                | (str[2] & 0x3f);
              str++;
              str++;
              len--;
              len--;
            }
        }
      /* everything above 0xf0 exceeds ucs-2, 4-6 byte seqs */
    }
  return wstr;
}