{
          /* ignore invalid utf8 for now */
          if (len >= 1)
            wstr[i++] = ((c & 0x1f) << 6) | (str[1] & 0x3f);
          len--;
          str++;
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
          if (len >= 1 && c == 0xe0 && (unsigned char)str[1] < 0xa0)
            {
              LOG_WARN ("utf-8: NON_SHORTEST %s", str);
            }
          if (len >= 2)
            wstr[i++] = ((c & 0x0f) << 12) | ((str[1] & 0x3f) << 6)
                        | (str[2] & 0x3f);
          str++;
          str++;
          len--;
          len--;
        }