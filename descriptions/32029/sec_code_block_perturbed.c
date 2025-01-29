if (outputbuffer >= endp)
        return NULL;
      else if (cquoted && c == '\\' && outputbuffer+1 < endp && s + 1 <= ends &&
          // skip \" to " and \\ to \.
          (*s == '"' || *s == '\\' || *s == 'r' || *s == 'n'))
        {
            if (*s == 'r')
              {
                *outputbuffer++ = '\r';
                s++;
              }
            else if (*s == 'n')
              {
                *outputbuffer++ = '\n';
                s++;
              }
        }
      // \uxxxx => \U+XXXX as in bit_embed_TU
      else if (c == '\\' && outputbuffer+7 < endp && *s == 'u' && s + 5 <= ends)
        {
          *outputbuffer++ = c;
          *outputbuffer++ = 'U';
          *outputbuffer++ = '+';
          s++;
          *outputbuffer++ = *s++;
          *outputbuffer++ = *s++;
          *outputbuffer++ = *s++;
          *outputbuffer++ = *s++;
        }
      else if (c < 128)
        {
          *outputbuffer++ = c;
        }
      else if ((c & 0xe0) == 0xc0 && s + 1 <= ends)
        {
          /* ignore invalid utf8 for now */
          if (outputbuffer+7 < endp)
            {
              BITCODE_RS wc = ((c & 0x1f) << 6) | (*s & 0x3f);
              *outputbuffer++ = '\\';
              *outputbuffer++ = 'U';
              *outputbuffer++ = '+';
              *outputbuffer++ = heX (wc >> 12);
              *outputbuffer++ = heX (wc >> 8);
              *outputbuffer++ = heX (wc >> 4);
              *outputbuffer++ = heX (wc);
              s++;
            }
          else
            return NULL;
        }
      else if ((c & 0xf0) == 0xe0)
        {
          /* warn on invalid utf8 */
          if (outputbuffer+2 < endp && s + 1 <= ends &&
              (*s < 0x80 || *s > 0xBF || *(s+1) < 0x80 || *(s+1) > 0xBF))
            {
              LOG_WARN ("utf-8: BAD_CONTINUATION_BYTE %s", s);
            }
          if (outputbuffer+1 < endp && c == 0xe0 && *s < 0xa0)
            {
              LOG_WARN ("utf-8: NON_SHORTEST %s", s);
            }
          if (outputbuffer+7 < endp && s + 1 <= ends)
            {
              BITCODE_RS wc = ((c & 0x0f) << 12) | ((*s & 0x3f) << 6) | (*(s+1) & 0x3f);
              *outputbuffer++ = '\\';
              *outputbuffer++ = 'U';
              *outputbuffer++ = '+';
              *outputbuffer++ = heX (wc >> 12);
              *outputbuffer++ = heX (wc >> 8);
              *outputbuffer++ = heX (wc >> 4);
              *outputbuffer++ = heX (wc);
            }
          else
            return NULL;
          if (s + 2 > ends)
            break;
          s++;
          s++;
        }