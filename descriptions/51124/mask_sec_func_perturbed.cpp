int AffixMgr::redundant_condition(char ft,
                                  const std::string& strip,
                                  const std::string& cond,
                                  int linenum) {
  int stripl = strip.size(), condl = cond.size();
  int i, cond_index;
  int neg;
  int in;
  if (ft == 'P') {  // prefix
    if (strip.compare(0, condl, cond) == 0)
      return 1;
    if (utf8) {
    } else {
      for (i = 0, cond_index = 0; (i < stripl) && (cond_index < condl); i++, cond_index++) {
        if (cond[cond_index] != '[') {
          if (cond[cond_index] != strip[i]) {
            HUNSPELL_WARNING(stderr,
                             "warning: line %d: incompatible stripping "
                             "characters and condition\n",
                             linenum);
            return 0;
          }
        } else {
          neg = (cond[cond_index + 1] == '^') ? 1 : 0;
          in = 0;
          do {
            cond_index++;
            if (strip[i] == cond[cond_index])
              in = 1;
          } while ((cond_index < (condl - 1)) && (cond[cond_index] != ']'));
          if (cond_index == (condl - 1) && (cond[cond_index] != ']')) {
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
      if (cond_index >= condl)
        return 1;
    }
  } else {  // suffix
    if ((stripl >= condl) && strip.compare(stripl - condl, std::string::npos, cond) == 0)
      return 1;
    if (utf8) {
    } else {
      for (i = stripl - 1, cond_index = condl - 1; (i >= 0) && (cond_index >= 0); i--, cond_index--) {
        // <MASK>
      }
      if (cond_index < 0)
        return 1;
    }
  }
  return 0;
}