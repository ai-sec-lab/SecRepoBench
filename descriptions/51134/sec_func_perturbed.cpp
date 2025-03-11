inline int PfxEntry::test_condition(const std::string& str) {
  size_t st = 0;
  size_t pos = std::string::npos;  // group with pos input position
  bool neg = false;        // complementer
  bool ingroup = false;    // character in the group
  if (numconds == 0)
    return 1;
  char* p = c.conds;
  while (1) {
    switch (*p) {
      case '\0':
        return 1;
      case '[': {
        neg = false;
        ingroup = false;
        p = nextchar(p);
        pos = st;
        break;
      }
      case '^': {
        p = nextchar(p);
        neg = true;
        break;
      }
      case ']': {
        if (bool(neg) == bool(ingroup))
          return 0;
        pos = std::string::npos;
        p = nextchar(p);
        // skip the next character
        if (!ingroup && st < str.size()) {
          ++st;
          while ((opts & aeUTF8) && st < str.size() && (str[st] & 0xc0) == 0x80)
            ++st;
        }
        if (st == str.size() && p)
          return 0;  // word <= condition
        break;
      }
      case '.':
        if (pos == std::string::npos) {  // dots are not metacharacters in groups: [.]
          p = nextchar(p);
          // skip the next character
          ++st;
          while ((opts & aeUTF8) && st < str.size() && (str[st] & 0xc0) == 0x80)
            ++st;
          if (st == str.size() && p)
            return 0;  // word <= condition
          break;
        }
      /* FALLTHROUGH */
      default: {
        if (st < str.size() && str[st] == *p) {
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
      }
    }
    if (!p)
      return 1;
  }
}