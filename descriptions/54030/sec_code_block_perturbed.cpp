// perhaps second word is a compound word (recursive call)
            if (wordnum + 2 < maxwordnum) {
              rv = compound_check(st.substr(i), wordnum + 1,
                                  numsyllable, maxwordnum, wnum + 1, words, rwords, 0,
                                  is_sug, info);

              if (rv && !checkcpdtable.empty() && i < word.size() &&
                  ((scpd == 0 &&
                    cpdpat_check(word, i, rv_first, rv, is_affixed)) ||
                   (scpd != 0 &&
                    !cpdpat_check(word, i, rv_first, rv, is_affixed))))
                rv = NULL;
            } else {
              rv = NULL;
            }