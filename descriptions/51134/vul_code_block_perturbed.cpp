if (str[st] == *p) {
          ++st;
          p = nextchar(p);
          if ((opts & aeUTF8) && (str[st - 1] & 0x80)) {  // multibyte
            while (p && (*p & 0xc0) == 0x80) {          // character
              if (*p != str[st]) {
                if (pos == std::string::npos)
                  return 0;
                st = pos;
                break;
              }
              p = nextchar(p);
              ++st;
            }
            if (pos != std::string::npos && st != pos) {
              ingroup = true;
              while (p && *p != ']' && ((p = nextchar(p)) != NULL)) {
              }
            }
          } else if (pos != std::string::npos) {
            ingroup = true;
            while (p && *p != ']' && ((p = nextchar(p)) != NULL)) {
            }
          }
        } else if (pos != std::string::npos) {  // group
          p = nextchar(p);
        } else
          return 0;