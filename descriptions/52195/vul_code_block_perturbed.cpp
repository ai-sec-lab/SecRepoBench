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

            // check FORCEUCASE
            if (rv && forceucase &&
                (TESTAFF(rv->astr, forceucase, rv->alen)) &&
                !(info && *info & SPELL_ORIGCAP))
              rv = NULL;

            if (rv && words && words[wnum + 1])
              return rv_first;

            oldnumsyllable2 = numsyllable;
            previouswordnum = wordnum;

            // LANG_hu section: spec. Hungarian rule, XXX hardwired dictionary
            // code
            if ((rv) && (langnum == LANG_hu) &&
                (TESTAFF(rv->astr, 'I', rv->alen)) &&
                !(TESTAFF(rv->astr, 'J', rv->alen))) {
              numsyllable--;
            }
            // END of LANG_hu section

            // increment word number, if the second root has a compoundroot flag
            if ((rv) && (compoundroot) &&
                (TESTAFF(rv->astr, compoundroot, rv->alen))) {
              wordnum++;
            }

            // check forbiddenwords
            if ((rv) && (rv->astr) &&
                (TESTAFF(rv->astr, forbiddenword, rv->alen) ||
                 TESTAFF(rv->astr, ONLYUPCASEFLAG, rv->alen) ||
                 (is_sug && nosuggest &&
                  TESTAFF(rv->astr, nosuggest, rv->alen))))
              return NULL;

            // second word is acceptable, as a root?
            // hungarian conventions: compounding is acceptable,
            // when compound forms consist of 2 words, or if more,
            // then the syllable number of root words must be 6, or lesser.

            if ((rv) &&
                ((compoundflag && TESTAFF(rv->astr, compoundflag, rv->alen)) ||
                 (compoundend && TESTAFF(rv->astr, compoundend, rv->alen))) &&
                (((cpdwordmax == -1) || (wordnum + 1 < cpdwordmax)) ||
                 ((cpdmaxsyllable != 0) &&
                  (numsyllable + get_syllable(std::string(HENTRY_WORD(rv), rv->blen)) <=
                   cpdmaxsyllable))) &&
                (
                    // test CHECKCOMPOUNDPATTERN
                    checkcpdtable.empty() || scpd != 0 ||
                    !cpdpat_check(word.c_str(), i, rv_first, rv, 0)) &&
                ((!checkcompounddup || (rv != rv_first)))
                // test CHECKCOMPOUNDPATTERN conditions
                &&
                (scpd == 0 || checkcpdtable[scpd - 1].cond2 == FLAG_NULL ||
                 TESTAFF(rv->astr, checkcpdtable[scpd - 1].cond2, rv->alen))) {
              // forbid compound word, if it is a non-compound word with typical
              // fault
              if ((checkcompoundrep && cpdrep_check(word, len)) ||
                      cpdwordpair_check(word, len))
                return NULL;
              return rv_first;
            }

            numsyllable = oldnumsyllable2;
            wordnum = previouswordnum;

            // perhaps second word has prefix or/and suffix
            sfx = NULL;
            sfxflag = FLAG_NULL;
            rv = (compoundflag && !onlycpdrule && i < word.size())
                     ? affix_check(word, i, word.size() - i, compoundflag,
                                   IN_CPD_END)
                     : NULL;
            if (!rv && compoundend && !onlycpdrule) {
              sfx = NULL;
              pfx = NULL;
              if (i < word.size())
                rv = affix_check(word, i, word.size() - i, compoundend, IN_CPD_END);
            }

            if (!rv && !defcpdtable.empty() && words) {
              if (i < word.size())
                rv = affix_check(word, i, word.size() - i, 0, IN_CPD_END);
              if (rv && defcpd_check(&words, wnum + 1, rv, NULL, 1))
                return rv_first;
              rv = NULL;
            }

            // test CHECKCOMPOUNDPATTERN conditions (allowed forms)
            if (rv &&
                !(scpd == 0 || checkcpdtable[scpd - 1].cond2 == FLAG_NULL ||
                  TESTAFF(rv->astr, checkcpdtable[scpd - 1].cond2, rv->alen)))
              rv = NULL;

            // test CHECKCOMPOUNDPATTERN conditions (forbidden compounds)
            if (rv && !checkcpdtable.empty() && scpd == 0 &&
                cpdpat_check(word.c_str(), i, rv_first, rv, affixed))
              rv = NULL;

            // check non_compound flag in suffix and prefix
            if ((rv) && ((pfx && pfx->getCont() &&
                          TESTAFF(pfx->getCont(), compoundforbidflag,
                                  pfx->getContLen())) ||
                         (sfx && sfx->getCont() &&
                          TESTAFF(sfx->getCont(), compoundforbidflag,
                                  sfx->getContLen())))) {
              rv = NULL;
            }

            // check FORCEUCASE
            if (rv && forceucase &&
                (TESTAFF(rv->astr, forceucase, rv->alen)) &&
                !(info && *info & SPELL_ORIGCAP))
              rv = NULL;

            // check forbiddenwords
            if ((rv) && (rv->astr) &&
                (TESTAFF(rv->astr, forbiddenword, rv->alen) ||
                 TESTAFF(rv->astr, ONLYUPCASEFLAG, rv->alen) ||
                 (is_sug && nosuggest &&
                  TESTAFF(rv->astr, nosuggest, rv->alen))))
              return NULL;

            // pfxappnd = prefix of word+i, or NULL
            // calculate syllable number of prefix.
            // hungarian convention: when syllable number of prefix is more,
            // than 1, the prefix+word counts as two words.

            if (langnum == LANG_hu) {
              // calculate syllable number of the word
              numsyllable += get_syllable(word.c_str() + i);

              // - affix syllable num.
              // XXX only second suffix (inflections, not derivations)
              if (sfxappnd) {
                std::string tmp(sfxappnd);
                reverseword(tmp);
                numsyllable -= short(get_syllable(tmp) + sfxextra);
              } else {
                numsyllable -= short(sfxextra);
              }

              // + 1 word, if syllable number of the prefix > 1 (hungarian
              // convention)
              if (pfx && (get_syllable(pfx->getKey()) > 1))
                wordnum++;

              // increment syllable num, if last word has a SYLLABLENUM flag
              // and the suffix is beginning `s'

              if (!cpdsyllablenum.empty()) {
                switch (sfxflag) {
                  case 'c': {
                    numsyllable += 2;
                    break;
                  }
                  case 'J': {
                    numsyllable += 1;
                    break;
                  }
                  case 'I': {
                    if (rv && TESTAFF(rv->astr, 'J', rv->alen))
                      numsyllable += 1;
                    break;
                  }
                }
              }
            }

            // increment word number, if the second word has a compoundroot flag
            if ((rv) && (compoundroot) &&
                (TESTAFF(rv->astr, compoundroot, rv->alen))) {
              wordnum++;
            }
            // second word is acceptable, as a word with prefix or/and suffix?
            // hungarian conventions: compounding is acceptable,
            // when compound forms consist 2 word, otherwise
            // the syllable number of root words is 6, or lesser.
            if ((rv) &&
                (((cpdwordmax == -1) || (wordnum + 1 < cpdwordmax)) ||
                 ((cpdmaxsyllable != 0) && (numsyllable <= cpdmaxsyllable))) &&
                ((!checkcompounddup || (rv != rv_first)))) {
              // forbid compound word, if it is a non-compound word with typical
              // fault
              if ((checkcompoundrep && cpdrep_check(word, len)) ||
                      cpdwordpair_check(word, len))
                return NULL;
              return rv_first;
            }

            numsyllable = oldnumsyllable2;
            wordnum = previouswordnum;

            // perhaps second word is a compound word (recursive call)
            if (wordnum + 2 < maxwordnum) {
              rv = compound_check(st.substr(i), wordnum + 1,
                                  numsyllable, maxwordnum, wnum + 1, words, rwords, 0,
                                  is_sug, info);

              if (rv && !checkcpdtable.empty() &&
                  ((scpd == 0 &&
                    cpdpat_check(word.c_str(), i, rv_first, rv, affixed)) ||
                   (scpd != 0 &&
                    !cpdpat_check(word.c_str(), i, rv_first, rv, affixed))))
                rv = NULL;
            } else {
              rv = NULL;
            }
            if (rv) {
              // forbid compound word, if it is a non-compound word with typical
              // fault, or a dictionary word pair

              if (cpdwordpair_check(word, len))
                  return NULL;

              if (checkcompoundrep || forbiddenword) {

                if (checkcompoundrep && cpdrep_check(word, len))
                  return NULL;

                // check first part
                if (word.compare(i, rv->blen, rv->word, rv->blen) == 0) {
                  char r = st[i + rv->blen];
                  st[i + rv->blen] = '\0';

                  if ((checkcompoundrep && cpdrep_check(st, i + rv->blen)) ||
                      cpdwordpair_check(st, i + rv->blen)) {
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
              }
              return rv_first;
            }