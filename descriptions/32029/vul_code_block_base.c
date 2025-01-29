if (dest >= endp)
        return NULL;
      else if (cquoted && c == '\\' && dest+1 < endp &&
          // skip \" to " and \\ to \.
          (*s == '"' || *s == '\\' || *s == 'r' || *s == 'n'))
        {
            if (*s == 'r')
              {
                *dest++ = '\r';
                s++;
              }
            else if (*s == 'n')
              {
                *dest++ = '\n';
                s++;
              }
        }
      // \uxxxx => \U+XXXX as in bit_embed_TU
      else if (c == '\\' && dest+7 < endp && *s == 'u')
        {
          *dest++ = c;
          *dest++ = 'U';
          *dest++ = '+';
          s++;
          *dest++ = *s++;
          *dest++ = *s++;
          *dest++ = *s++;
          *dest++ = *s++;
        }
      else if (c < 128)
        {
          *dest++ = c;
        }
      else if ((c & 0xe0) == 0xc0)
        {
          /* ignore invalid utf8 for now */
          if (dest+7 < endp)
            {
              BITCODE_RS wc = ((c & 0x1f) << 6) | (*s & 0x3f);
              *dest++ = '\\';
              *dest++ = 'U';
              *dest++ = '+';
              *dest++ = heX (wc >> 12);
              *dest++ = heX (wc >> 8);
              *dest++ = heX (wc >> 4);
              *dest++ = heX (wc);
              s++;
            }
          else
            return NULL;
        }
      else if ((c & 0xf0) == 0xe0)
        {
          /* warn on invalid utf8 */
          if (dest+2 < endp &&
              (*s < 0x80 || *s > 0xBF || *(s+1) < 0x80 || *(s+1) > 0xBF))
            {
              LOG_WARN ("utf-8: BAD_CONTINUATION_BYTE %s", s);
            }
          if (dest+1 < endp && c == 0xe0 && *s < 0xa0)
            {
              LOG_WARN ("utf-8: NON_SHORTEST %s", s);
            }
          if (dest+7 < endp)
            {
              BITCODE_RS wc = ((c & 0x0f) << 12) | ((*s & 0x3f) << 6) | (*(s+1) & 0x3f);
              *dest++ = '\\';
              *dest++ = 'U';
              *dest++ = '+';
              *dest++ = heX (wc >> 12);
              *dest++ = heX (wc >> 8);
              *dest++ = heX (wc >> 4);
              *dest++ = heX (wc);
            }
          else
            return NULL;
          s++;
          s++;
        }