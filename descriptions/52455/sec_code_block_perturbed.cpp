if ((checkcompoundrep && cpdrep_check(st, i + compound_result->blen)) ||
                      cpdwordpair_check(st, i + compound_result->blen)) {
                    st[ + i + compound_result->blen] = r;
                    continue;
                  }

                  if (forbiddenword) {
                    struct hentry* rv2 = lookup(word.c_str(), word.size());
                    if (!rv2 && len <= word.size())
                      rv2 = affix_check(word, 0, len);
                    if (rv2 && rv2->astr &&
                        TESTAFF(rv2->astr, forbiddenword, rv2->alen) &&
                        (strncmp(rv2->word, st.c_str(), i + compound_result->blen) == 0)) {
                      return NULL;
                    }
                  }
                  st[i + compound_result->blen] = r;