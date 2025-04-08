inline int PfxEntry::test_condition(const std::string& s) {
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
        if (!ingroup && st < s.size()) {
          ++st;
          while ((opts & aeUTF8) && st < s.size() && (s[st] & 0xc0) == 0x80)
            ++st;
        }
        if (st == s.size() && p)
          return 0;  // word <= condition
        break;
      }
      case '.':
        if (pos == std::string::npos) {  // dots are not metacharacters in groups: [.]
          p = nextchar(p);
          // skip the next character
          ++st;
          while ((opts & aeUTF8) && st < s.size() && (s[st] & 0xc0) == 0x80)
            ++st;
          if (st == s.size() && p)
            return 0;  // word <= condition
          break;
        }
      /* FALLTHROUGH */
      default: {
        // Compare the current character in the input string with the current condition character.
        // If they match, increment the position in the input string and advance to the next condition character.
        // Handle multibyte UTF-8 characters if the option is set, ensuring the entire character sequence matches.
        // If the input character matches within a group, mark that the character was found in the group.
        // If the character does not match and is not within a group, return 0 to indicate the condition failed.
        // Continue processing the condition string or group until the end of the string or group.
        // <MASK>
      }
    }
    if (!p)
      return 1;
  }
}