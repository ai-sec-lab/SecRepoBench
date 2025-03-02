if (cond[j] != ']') {
          if (cond[j] != strip[i]) {
            HUNSPELL_WARNING(stderr,
                             "warning: line %d: incompatible stripping "
                             "characters and condition\n",
                             linenum);
            return 0;
          }
        } else {
          in = 0;
          do {
            j--;
            if (strip[i] == cond[j])
              in = 1;
          } while ((j > 0) && (cond[j] != '['));
          if ((j == 0) && (cond[j] != '[')) {
            HUNSPELL_WARNING(stderr,
                             "error: line: %d: missing ] in condition:\n%s\n",
                             linenum, cond.c_str());
            return 0;
          }
          neg = (cond[j + 1] == '^') ? 1 : 0;
          if ((!neg && !in) || (neg && in)) {
            HUNSPELL_WARNING(stderr,
                             "warning: line %d: incompatible stripping "
                             "characters and condition\n",
                             linenum);
            return 0;
          }
        }