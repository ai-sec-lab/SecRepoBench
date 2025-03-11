struct hentry* AffixMgr::compound_check(const std::string& word,
                                        short wordnum,
                                        short numsyllable,
                                        short maxwordnum,
                                        short wnum,
                                        hentry** words = NULL,
                                        hentry** rwords = NULL,
                                        char hu_mov_rule = 0,
                                        char is_sug = 0,
                                        int* info = NULL) {
  short oldnumsyllable, oldnumsyllable2, oldwordnum, oldwordnum2;
  hentry *compound_result = NULL, *rv_first;
  std::string st;
  char ch = '\0', affixed;
  int cmin, cmax, striple = 0, soldi = 0, oldcmin = 0, oldcmax = 0, oldlen = 0, checkedstriple = 0;
  hentry** oldwords = words;
  size_t scpd = 0, len = word.size();

  int checked_prefix;

  // add a time limit to handle possible
  // combinatorical explosion of the overlapping words

  HUNSPELL_THREAD_LOCAL clock_t timelimit;

  if (wordnum == 0) {
      // get the start time, seeing as we're reusing this set to 0
      // to flag timeout, use clock() + 1 to avoid start clock()
      // of 0 as being a timeout
      timelimit = clock() + 1;
  }
  else if (timelimit != 0 && (clock() > timelimit + TIMELIMIT)) {
      timelimit = 0;
  }

  setcminmax(&cmin, &cmax, word.c_str(), len);

  st.assign(word);

  for (int i = cmin; i < cmax; ++i) {
    // go to end of the UTF-8 character
    if (utf8) {
      for (; (st[i] & 0xc0) == 0x80; i++)
        ;
      if (i >= cmax)
        return NULL;
    }

    words = oldwords;
    int onlycpdrule = (words) ? 1 : 0;

    do {  // onlycpdrule loop

      oldnumsyllable = numsyllable;
      oldwordnum = wordnum;
      checked_prefix = 0;

      do {  // simplified checkcompoundpattern loop

        if (timelimit == 0)
          return 0;

        if (scpd > 0) {
          for (; scpd <= checkcpdtable.size() &&
                 (checkcpdtable[scpd - 1].pattern3.empty() ||
                  i > word.size() ||
                  word.compare(i, checkcpdtable[scpd - 1].pattern3.size(), checkcpdtable[scpd - 1].pattern3) != 0);
               scpd++)
            ;

          if (scpd > checkcpdtable.size())
            break;  // break simplified checkcompoundpattern loop
          st.replace(i, std::string::npos, checkcpdtable[scpd - 1].pattern);
          soldi = i;
          i += checkcpdtable[scpd - 1].pattern.size();
          st.replace(i, std::string::npos, checkcpdtable[scpd - 1].pattern2);
          st.replace(i + checkcpdtable[scpd - 1].pattern2.size(), std::string::npos,
                 word.substr(soldi + checkcpdtable[scpd - 1].pattern3.size()));

          oldlen = len;
          len += checkcpdtable[scpd - 1].pattern.size() +
                 checkcpdtable[scpd - 1].pattern2.size() -
                 checkcpdtable[scpd - 1].pattern3.size();
          oldcmin = cmin;
          oldcmax = cmax;
          setcminmax(&cmin, &cmax, st.c_str(), len);

          cmax = len - cpdmin + 1;
        }

	if (i > st.size())
	    return NULL;

        ch = st[i];
        st[i] = '\0';

        sfx = NULL;
        pfx = NULL;

        // FIRST WORD

        affixed = 1;
        compound_result = lookup(st.c_str(), i);  // perhaps without prefix

        // forbid dictionary stems with COMPOUNDFORBIDFLAG in
        // compound words, overriding the effect of COMPOUNDPERMITFLAG
        if ((compound_result) && compoundforbidflag &&
                TESTAFF(compound_result->astr, compoundforbidflag, compound_result->alen) && !hu_mov_rule)
            continue;

        // search homonym with compound flag
        while ((compound_result) && !hu_mov_rule &&
               ((needaffix && TESTAFF(compound_result->astr, needaffix, compound_result->alen)) ||
                !((compoundflag && !words && !onlycpdrule &&
                   TESTAFF(compound_result->astr, compoundflag, compound_result->alen)) ||
                  (compoundbegin && !wordnum && !onlycpdrule &&
                   TESTAFF(compound_result->astr, compoundbegin, compound_result->alen)) ||
                  (compoundmiddle && wordnum && !words && !onlycpdrule &&
                   TESTAFF(compound_result->astr, compoundmiddle, compound_result->alen)) ||
                  (!defcpdtable.empty() && onlycpdrule &&
                   ((!words && !wordnum &&
                     defcpd_check(&words, wnum, compound_result, rwords, 0)) ||
                    (words &&
                     defcpd_check(&words, wnum, compound_result, rwords, 0))))) ||
                (scpd != 0 && checkcpdtable[scpd - 1].cond != FLAG_NULL &&
                 !TESTAFF(compound_result->astr, checkcpdtable[scpd - 1].cond, compound_result->alen)))) {
          compound_result = compound_result->next_homonym;
        }

        if (compound_result)
          affixed = 0;

        if (!compound_result) {
          if (onlycpdrule)
            break;
          if (compoundflag &&
              !(compound_result = prefix_check(st, 0, i,
                                  hu_mov_rule ? IN_CPD_OTHER : IN_CPD_BEGIN,
                                  compoundflag))) {
            if (((compound_result = suffix_check(
                      st, 0, i, 0, NULL, FLAG_NULL, compoundflag,
                      hu_mov_rule ? IN_CPD_OTHER : IN_CPD_BEGIN)) ||
                 (compoundmoresuffixes &&
                  (compound_result = suffix_check_twosfx(st, 0, i, 0, NULL, compoundflag)))) &&
                !hu_mov_rule && sfx->getCont() &&
                ((compoundforbidflag &&
                  TESTAFF(sfx->getCont(), compoundforbidflag,
                          sfx->getContLen())) ||
                 (compoundend &&
                  TESTAFF(sfx->getCont(), compoundend, sfx->getContLen())))) {
              compound_result = NULL;
            }
          }

          if (compound_result ||
              (((wordnum == 0) && compoundbegin &&
                ((compound_result = suffix_check(
                      st, 0, i, 0, NULL, FLAG_NULL, compoundbegin,
                      hu_mov_rule ? IN_CPD_OTHER : IN_CPD_BEGIN)) ||
                 (compoundmoresuffixes &&
                  (compound_result = suffix_check_twosfx(
                       st, 0, i, 0, NULL,
                       compoundbegin))) ||  // twofold suffixes + compound
                 (compound_result = prefix_check(st, 0, i,
                                    hu_mov_rule ? IN_CPD_OTHER : IN_CPD_BEGIN,
                                    compoundbegin)))) ||
               ((wordnum > 0) && compoundmiddle &&
                ((compound_result = suffix_check(
                      st, 0, i, 0, NULL, FLAG_NULL, compoundmiddle,
                      hu_mov_rule ? IN_CPD_OTHER : IN_CPD_BEGIN)) ||
                 (compoundmoresuffixes &&
                  (compound_result = suffix_check_twosfx(
                       st, 0, i, 0, NULL,
                       compoundmiddle))) ||  // twofold suffixes + compound
                 (compound_result = prefix_check(st, 0, i,
                                    hu_mov_rule ? IN_CPD_OTHER : IN_CPD_BEGIN,
                                    compoundmiddle))))))
            checked_prefix = 1;
          // else check forbiddenwords and needaffix
        } else if (compound_result->astr && (TESTAFF(compound_result->astr, forbiddenword, compound_result->alen) ||
                                TESTAFF(compound_result->astr, needaffix, compound_result->alen) ||
                                TESTAFF(compound_result->astr, ONLYUPCASEFLAG, compound_result->alen) ||
                                (is_sug && nosuggest &&
                                 TESTAFF(compound_result->astr, nosuggest, compound_result->alen)))) {
          st[i] = ch;
          // continue;
          break;
        }

        // check non_compound flag in suffix and prefix
        if ((compound_result) && !hu_mov_rule &&
            ((pfx && pfx->getCont() &&
              TESTAFF(pfx->getCont(), compoundforbidflag, pfx->getContLen())) ||
             (sfx && sfx->getCont() &&
              TESTAFF(sfx->getCont(), compoundforbidflag,
                      sfx->getContLen())))) {
          compound_result = NULL;
        }

        // check compoundend flag in suffix and prefix
        if ((compound_result) && !checked_prefix && compoundend && !hu_mov_rule &&
            ((pfx && pfx->getCont() &&
              TESTAFF(pfx->getCont(), compoundend, pfx->getContLen())) ||
             (sfx && sfx->getCont() &&
              TESTAFF(sfx->getCont(), compoundend, sfx->getContLen())))) {
          compound_result = NULL;
        }

        // check compoundmiddle flag in suffix and prefix
        if ((compound_result) && !checked_prefix && (wordnum == 0) && compoundmiddle &&
            !hu_mov_rule &&
            ((pfx && pfx->getCont() &&
              TESTAFF(pfx->getCont(), compoundmiddle, pfx->getContLen())) ||
             (sfx && sfx->getCont() &&
              TESTAFF(sfx->getCont(), compoundmiddle, sfx->getContLen())))) {
          compound_result = NULL;
        }

        // check forbiddenwords
        if ((compound_result) && (compound_result->astr) &&
            (TESTAFF(compound_result->astr, forbiddenword, compound_result->alen) ||
             TESTAFF(compound_result->astr, ONLYUPCASEFLAG, compound_result->alen) ||
             (is_sug && nosuggest && TESTAFF(compound_result->astr, nosuggest, compound_result->alen)))) {
          return NULL;
        }

        // increment word number, if the second root has a compoundroot flag
        if ((compound_result) && compoundroot &&
            (TESTAFF(compound_result->astr, compoundroot, compound_result->alen))) {
          wordnum++;
        }

        // first word is acceptable in compound words?
        if (((compound_result) &&
             (checked_prefix || (words && words[wnum]) ||
              (compoundflag && TESTAFF(compound_result->astr, compoundflag, compound_result->alen)) ||
              ((oldwordnum == 0) && compoundbegin &&
               TESTAFF(compound_result->astr, compoundbegin, compound_result->alen)) ||
              ((oldwordnum > 0) && compoundmiddle &&
               TESTAFF(compound_result->astr, compoundmiddle, compound_result->alen))

              // LANG_hu section: spec. Hungarian rule
              || ((langnum == LANG_hu) && hu_mov_rule &&
                  (TESTAFF(
                       compound_result->astr, 'F',
                       compound_result->alen) ||  // XXX hardwired Hungarian dictionary codes
                   TESTAFF(compound_result->astr, 'G', compound_result->alen) ||
                   TESTAFF(compound_result->astr, 'H', compound_result->alen)))
              // END of LANG_hu section
              ) &&
             (
                 // test CHECKCOMPOUNDPATTERN conditions
                 scpd == 0 || checkcpdtable[scpd - 1].cond == FLAG_NULL ||
                 TESTAFF(compound_result->astr, checkcpdtable[scpd - 1].cond, compound_result->alen)) &&
             !((checkcompoundtriple && scpd == 0 &&
                !words &&  // test triple letters
                (word[i - 1] == word[i]) &&
                (((i > 1) && (word[i - 1] == word[i - 2])) ||
                 ((word[i - 1] == word[i + 1]))  // may be word[i+1] == '\0'
                 )) ||
               (checkcompoundcase && scpd == 0 && !words &&
                cpdcase_check(word, i))))
            // LANG_hu section: spec. Hungarian rule
            || ((!compound_result) && (langnum == LANG_hu) && hu_mov_rule &&
                (compound_result = affix_check(st, 0, i)) &&
                (sfx && sfx->getCont() &&
                 (  // XXX hardwired Hungarian dic. codes
                     TESTAFF(sfx->getCont(), (unsigned short)'x',
                             sfx->getContLen()) ||
                     TESTAFF(
                         sfx->getCont(), (unsigned short)'%',
                         sfx->getContLen()))))) {  // first word is ok condition

          // LANG_hu section: spec. Hungarian rule
          if (langnum == LANG_hu) {
            // calculate syllable number of the word
            numsyllable += get_syllable(st.substr(0, i));
            // + 1 word, if syllable number of the prefix > 1 (hungarian
            // convention)
            if (pfx && (get_syllable(pfx->getKey()) > 1))
              wordnum++;
          }
          // END of LANG_hu section

          // NEXT WORD(S)
          rv_first = compound_result;
          st[i] = ch;

          do {  // striple loop

            // check simplifiedtriple
            if (simplifiedtriple) {
              if (striple) {
                checkedstriple = 1;
                i--;  // check "fahrt" instead of "ahrt" in "Schiffahrt"
              } else if (i > 2 && i <= word.size() && word[i - 1] == word[i - 2])
                striple = 1;
            }

            compound_result = lookup(st.c_str() + i, st.size() - i);  // perhaps without prefix

            // search homonym with compound flag
            while ((compound_result) &&
                   ((needaffix && TESTAFF(compound_result->astr, needaffix, compound_result->alen)) ||
                    !((compoundflag && !words &&
                       TESTAFF(compound_result->astr, compoundflag, compound_result->alen)) ||
                      (compoundend && !words &&
                       TESTAFF(compound_result->astr, compoundend, compound_result->alen)) ||
                      (!defcpdtable.empty() && words &&
                       defcpd_check(&words, wnum + 1, compound_result, NULL, 1))) ||
                    (scpd != 0 && checkcpdtable[scpd - 1].cond2 != FLAG_NULL &&
                     !TESTAFF(compound_result->astr, checkcpdtable[scpd - 1].cond2,
                              compound_result->alen)))) {
              compound_result = compound_result->next_homonym;
            }

            // check FORCEUCASE
            if (compound_result && forceucase &&
                (TESTAFF(compound_result->astr, forceucase, compound_result->alen)) &&
                !(info && *info & SPELL_ORIGCAP))
              compound_result = NULL;

            if (compound_result && words && words[wnum + 1])
              return rv_first;

            oldnumsyllable2 = numsyllable;
            oldwordnum2 = wordnum;

            // LANG_hu section: spec. Hungarian rule, XXX hardwired dictionary
            // code
            if ((compound_result) && (langnum == LANG_hu) &&
                (TESTAFF(compound_result->astr, 'I', compound_result->alen)) &&
                !(TESTAFF(compound_result->astr, 'J', compound_result->alen))) {
              numsyllable--;
            }
            // END of LANG_hu section

            // increment word number, if the second root has a compoundroot flag
            if ((compound_result) && (compoundroot) &&
                (TESTAFF(compound_result->astr, compoundroot, compound_result->alen))) {
              wordnum++;
            }

            // check forbiddenwords
            if ((compound_result) && (compound_result->astr) &&
                (TESTAFF(compound_result->astr, forbiddenword, compound_result->alen) ||
                 TESTAFF(compound_result->astr, ONLYUPCASEFLAG, compound_result->alen) ||
                 (is_sug && nosuggest &&
                  TESTAFF(compound_result->astr, nosuggest, compound_result->alen))))
              return NULL;

            // second word is acceptable, as a root?
            // hungarian conventions: compounding is acceptable,
            // when compound forms consist of 2 words, or if more,
            // then the syllable number of root words must be 6, or lesser.

            if ((compound_result) &&
                ((compoundflag && TESTAFF(compound_result->astr, compoundflag, compound_result->alen)) ||
                 (compoundend && TESTAFF(compound_result->astr, compoundend, compound_result->alen))) &&
                (((cpdwordmax == -1) || (wordnum + 1 < cpdwordmax)) ||
                 ((cpdmaxsyllable != 0) &&
                  (numsyllable + get_syllable(std::string(HENTRY_WORD(compound_result), compound_result->blen)) <=
                   cpdmaxsyllable))) &&
                (
                    // test CHECKCOMPOUNDPATTERN
                    checkcpdtable.empty() || scpd != 0 ||
                    (i < word.size() && !cpdpat_check(word, i, rv_first, compound_result, 0))) &&
                ((!checkcompounddup || (compound_result != rv_first)))
                // test CHECKCOMPOUNDPATTERN conditions
                &&
                (scpd == 0 || checkcpdtable[scpd - 1].cond2 == FLAG_NULL ||
                 TESTAFF(compound_result->astr, checkcpdtable[scpd - 1].cond2, compound_result->alen))) {
              // forbid compound word, if it is a non-compound word with typical
              // fault
              if ((checkcompoundrep && cpdrep_check(word, len)) ||
                      cpdwordpair_check(word, len))
                return NULL;
              return rv_first;
            }

            numsyllable = oldnumsyllable2;
            wordnum = oldwordnum2;

            // perhaps second word has prefix or/and suffix
            sfx = NULL;
            sfxflag = FLAG_NULL;
            compound_result = (compoundflag && !onlycpdrule && i < word.size())
                     ? affix_check(word, i, word.size() - i, compoundflag,
                                   IN_CPD_END)
                     : NULL;
            if (!compound_result && compoundend && !onlycpdrule) {
              sfx = NULL;
              pfx = NULL;
              if (i < word.size())
                compound_result = affix_check(word, i, word.size() - i, compoundend, IN_CPD_END);
            }

            if (!compound_result && !defcpdtable.empty() && words) {
              if (i < word.size())
                compound_result = affix_check(word, i, word.size() - i, 0, IN_CPD_END);
              if (compound_result && defcpd_check(&words, wnum + 1, compound_result, NULL, 1))
                return rv_first;
              compound_result = NULL;
            }

            // test CHECKCOMPOUNDPATTERN conditions (allowed forms)
            if (compound_result &&
                !(scpd == 0 || checkcpdtable[scpd - 1].cond2 == FLAG_NULL ||
                  TESTAFF(compound_result->astr, checkcpdtable[scpd - 1].cond2, compound_result->alen)))
              compound_result = NULL;

            // test CHECKCOMPOUNDPATTERN conditions (forbidden compounds)
            if (compound_result && !checkcpdtable.empty() && scpd == 0 &&
                cpdpat_check(word, i, rv_first, compound_result, affixed))
              compound_result = NULL;

            // check non_compound flag in suffix and prefix
            if ((compound_result) && ((pfx && pfx->getCont() &&
                          TESTAFF(pfx->getCont(), compoundforbidflag,
                                  pfx->getContLen())) ||
                         (sfx && sfx->getCont() &&
                          TESTAFF(sfx->getCont(), compoundforbidflag,
                                  sfx->getContLen())))) {
              compound_result = NULL;
            }

            // check FORCEUCASE
            if (compound_result && forceucase &&
                (TESTAFF(compound_result->astr, forceucase, compound_result->alen)) &&
                !(info && *info & SPELL_ORIGCAP))
              compound_result = NULL;

            // check forbiddenwords
            if ((compound_result) && (compound_result->astr) &&
                (TESTAFF(compound_result->astr, forbiddenword, compound_result->alen) ||
                 TESTAFF(compound_result->astr, ONLYUPCASEFLAG, compound_result->alen) ||
                 (is_sug && nosuggest &&
                  TESTAFF(compound_result->astr, nosuggest, compound_result->alen))))
              return NULL;

            // pfxappnd = prefix of word+i, or NULL
            // calculate syllable number of prefix.
            // hungarian convention: when syllable number of prefix is more,
            // than 1, the prefix+word counts as two words.

            if (langnum == LANG_hu) {
              if (i < word.size()) {
                // calculate syllable number of the word
                numsyllable += get_syllable(word.substr(i));
              }

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
                    if (compound_result && TESTAFF(compound_result->astr, 'J', compound_result->alen))
                      numsyllable += 1;
                    break;
                  }
                }
              }
            }

            // increment word number, if the second word has a compoundroot flag
            if ((compound_result) && (compoundroot) &&
                (TESTAFF(compound_result->astr, compoundroot, compound_result->alen))) {
              wordnum++;
            }
            // second word is acceptable, as a word with prefix or/and suffix?
            // hungarian conventions: compounding is acceptable,
            // when compound forms consist 2 word, otherwise
            // the syllable number of root words is 6, or lesser.
            if ((compound_result) &&
                (((cpdwordmax == -1) || (wordnum + 1 < cpdwordmax)) ||
                 ((cpdmaxsyllable != 0) && (numsyllable <= cpdmaxsyllable))) &&
                ((!checkcompounddup || (compound_result != rv_first)))) {
              // forbid compound word, if it is a non-compound word with typical
              // fault
              if ((checkcompoundrep && cpdrep_check(word, len)) ||
                      cpdwordpair_check(word, len))
                return NULL;
              return rv_first;
            }

            numsyllable = oldnumsyllable2;
            wordnum = oldwordnum2;

            // perhaps second word is a compound word (recursive call)
            if (wordnum + 2 < maxwordnum) {
              compound_result = compound_check(st.substr(i), wordnum + 1,
                                  numsyllable, maxwordnum, wnum + 1, words, rwords, 0,
                                  is_sug, info);

              if (compound_result && !checkcpdtable.empty() &&
                  ((scpd == 0 &&
                    cpdpat_check(word, i, rv_first, compound_result, affixed)) ||
                   (scpd != 0 &&
                    !cpdpat_check(word, i, rv_first, compound_result, affixed))))
                compound_result = NULL;
            } else {
              compound_result = NULL;
            }
            if (compound_result) {
              // forbid compound word, if it is a non-compound word with typical
              // fault, or a dictionary word pair

              if (cpdwordpair_check(word, len))
                  return NULL;

              if (checkcompoundrep || forbiddenword) {

                if (checkcompoundrep && cpdrep_check(word, len))
                  return NULL;

                // check first part
                if (word.compare(i, compound_result->blen, compound_result->word, compound_result->blen) == 0) {
                  char r = st[i + compound_result->blen];
                  st[i + compound_result->blen] = '\0';

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
                }
              }
              return rv_first;
            }
          } while (striple && !checkedstriple);  // end of striple loop

          if (checkedstriple) {
            i++;
            checkedstriple = 0;
            striple = 0;
          }

        }  // first word is ok condition

        if (soldi != 0) {
          i = soldi;
          soldi = 0;
          len = oldlen;
          cmin = oldcmin;
          cmax = oldcmax;
        }
        scpd++;

      } while (!onlycpdrule && simplifiedcpd &&
               scpd <= checkcpdtable.size());  // end of simplifiedcpd loop

      scpd = 0;
      wordnum = oldwordnum;
      numsyllable = oldnumsyllable;

      if (soldi != 0) {
        i = soldi;
        st.assign(word);  // XXX add more optim.
        soldi = 0;
        len = oldlen;
        cmin = oldcmin;
        cmax = oldcmax;
      } else
        st[i] = ch;

    } while (!defcpdtable.empty() && oldwordnum == 0 &&
             onlycpdrule++ < 1);  // end of onlycpd loop
  }

  return NULL;
}