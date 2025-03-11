if (checkcompoundrep && cpdrep_check(compound_word.c_str(), len))
                  return NULL;

                // check first part
                if (compound_word.compare(i, rv->blen, rv->word, rv->blen) == 0) {
                  char r = st[i + rv->blen];
                  st[i + rv->blen] = '\0';

                  if ((checkcompoundrep && cpdrep_check(st.c_str(), i + rv->blen)) ||
                      cpdwordpair_check(st.c_str(), i + rv->blen)) {
                    st[ + i + rv->blen] = r;
                    continue;
                  }

                  if (forbiddenword) {
                    struct hentry* rv2 = lookup(compound_word.c_str(), compound_word.size());
                    if (!rv2)
                      rv2 = affix_check(compound_word, 0, len);
                    if (rv2 && rv2->astr &&
                        TESTAFF(rv2->astr, forbiddenword, rv2->alen) &&
                        (strncmp(rv2->word, st.c_str(), i + rv->blen) == 0)) {
                      return NULL;
                    }
                  }
                  st[i + rv->blen] = r;
                }