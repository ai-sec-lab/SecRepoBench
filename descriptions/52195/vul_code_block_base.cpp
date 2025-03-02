// striple loop

            // check simplifiedtriple
            if (simplifiedtriple) {
              if (striple) {
                checkedstriple = 1;
                i--;  // check "fahrt" instead of "ahrt" in "Schiffahrt"
              } else if (i > 2 && word[i - 1] == word[i - 2])
                striple = 1;
            }

            rv = lookup(st.c_str() + i, st.size() - i);  // perhaps without prefix

            // search homonym with compound flag
            while ((rv) &&
                   ((needaffix && TESTAFF(rv->astr, needaffix, rv->alen)) ||
                    !((compoundflag && !words &&
                       TESTAFF(rv->astr, compoundflag, rv->alen)) ||
                      (compoundend && !words &&
                       TESTAFF(rv->astr, compoundend, rv->alen)) ||
                      (!defcpdtable.empty() && words &&
                       defcpd_check(&words, wnum + 1, rv, NULL, 1))) ||
                    (scpd != 0 && checkcpdtable[scpd - 1].cond2 != FLAG_NULL &&
                     !TESTAFF(rv->astr, checkcpdtable[scpd - 1].cond2,
                              rv->alen)))) {
              rv = rv->next_homonym;
            }