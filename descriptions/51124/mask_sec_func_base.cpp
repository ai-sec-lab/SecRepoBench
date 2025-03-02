int AffixMgr::redundant_condition(char ft,
                                  const std::string& strip,
                                  const std::string& cond,
                                  int linenum) {
  int stripl = strip.size(), condl = cond.size();
  int i, j;
  int neg;
  int in;
  if (ft == 'P') {  // prefix
    if (strip.compare(0, condl, cond) == 0)
      return 1;
    if (utf8) {
    } else {
      for (i = 0, j = 0; (i < stripl) && (j < condl); i++, j++) {
        if (cond[j] != '[') {
          if (cond[j] != strip[i]) {
            HUNSPELL_WARNING(stderr,
                             "warning: line %d: incompatible stripping "
                             "characters and condition\n",
                             linenum);
            return 0;
          }
        } else {
          neg = (cond[j + 1] == '^') ? 1 : 0;
          in = 0;
          do {
            j++;
            if (strip[i] == cond[j])
              in = 1;
          } while ((j < (condl - 1)) && (cond[j] != ']'));
          if (j == (condl - 1) && (cond[j] != ']')) {
            HUNSPELL_WARNING(stderr,
                             "error: line %d: missing ] in condition:\n%s\n",
                             linenum, cond.c_str());
            return 0;
          }
          if ((!neg && !in) || (neg && in)) {
            HUNSPELL_WARNING(stderr,
                             "warning: line %d: incompatible stripping "
                             "characters and condition\n",
                             linenum);
            return 0;
          }
        }
      }
      if (j >= condl)
        return 1;
    }
  } else {  // suffix
    if ((stripl >= condl) && strip.compare(stripl - condl, std::string::npos, cond) == 0)
      return 1;
    if (utf8) {
    } else {
      for (i = stripl - 1, j = condl - 1; (i >= 0) && (j >= 0); i--, j--) {
        // <MASK>
      }
      if (j < 0)
        return 1;
    }
  }
  return 0;
}