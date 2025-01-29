if (checkcompoundrep && cpdrep_check(word.c_str(), len))
                  return NULL;

                // check first part
                if (strncmp(rv->word, word.c_str() + i, rv->blen) == 0) {
                  char r = st[i + rv->blen];
                  st[i + rv->blen] = '\0';

                  if ((checkcompoundrep && cpdrep_check(st.c_str(), i + rv->blen)) ||
                      cpdwordpair_check(st.c_str(), i + rv->blen)) {
                    st[ + i + rv->blen] = r;
                    continue;
                  }

                  if (forbiddenword) {
                    struct hentry* rv2 = lookup(word.c_str(), word.size());
                    if (!rv2)
                      rv2 = affix_check(word, 0, len);
                    if (rv2 && rv2->astr &&
                        TESTAFF(rv2->astr, forbiddenword, rv2->alen) &&
                        (strncmp(rv2->word, st.c_str(), i + rv->blen) == 0)) {
                      return NULL;
                    }
                  }
                  st[i + rv->blen] = r;
                }