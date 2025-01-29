rv = compound_check(st.substr(i), wordnum + 1,
                                  numsyllable, maxwordnum, wnum + 1, words, rwords, 0,
                                  is_sug, info);

              if (rv && !checkcpdtable.empty() && i < word.size() &&
                  ((scpd == 0 &&
                    cpdpat_check(word, i, rv_first, rv, affixed)) ||
                   (scpd != 0 &&
                    !cpdpat_check(word, i, rv_first, rv, affixed))))
                rv = NULL;