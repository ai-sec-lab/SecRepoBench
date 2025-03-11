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
  hentry *entry_result = NULL, *rv_first;
  std::string st;
  char ch = '\0', affixed;
  size_t cmin, cmax;
  int striple = 0, soldi = 0, oldcmin = 0, oldcmax = 0, oldlen = 0, checkedstriple = 0;
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

  for (size_t i = cmin; i < cmax; ++i) {
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
        entry_result = lookup(st.c_str(), i);  // perhaps without prefix

        // <MASK>

        if (entry_result)
          affixed = 0;

        if (!entry_result) {
          if (onlycpdrule)
            break;
          if (compoundflag &&
              !(entry_result = prefix_check(st, 0, i,
                                  hu_mov_rule ? IN_CPD_OTHER : IN_CPD_BEGIN,
                                  compoundflag))) {
            if (((entry_result = suffix_check(
                      st, 0, i, 0, NULL, FLAG_NULL, compoundflag,
                      hu_mov_rule ? IN_CPD_OTHER : IN_CPD_BEGIN)) ||
                 (compoundmoresuffixes &&
                  (entry_result = suffix_check_twosfx(st, 0, i, 0, NULL, compoundflag)))) &&
                !hu_mov_rule && sfx->getCont() &&
                ((compoundforbidflag &&
                  TESTAFF(sfx->getCont(), compoundforbidflag,
                          sfx->getContLen())) ||
                 (compoundend &&
                  TESTAFF(sfx->getCont(), compoundend, sfx->getContLen())))) {
              entry_result = NULL;
            }
          }

          if (entry_result ||
              (((wordnum == 0) && compoundbegin &&
                ((entry_result = suffix_check(
                      st, 0, i, 0, NULL, FLAG_NULL, compoundbegin,
                      hu_mov_rule ? IN_CPD_OTHER : IN_CPD_BEGIN)) ||
                 (compoundmoresuffixes &&
                  (entry_result = suffix_check_twosfx(
                       st, 0, i, 0, NULL,
                       compoundbegin))) ||  // twofold suffixes + compound
                 (entry_result = prefix_check(st, 0, i,
                                    hu_mov_rule ? IN_CPD_OTHER : IN_CPD_BEGIN,
                                    compoundbegin)))) ||
               ((wordnum > 0) && compoundmiddle &&
                ((entry_result = suffix_check(
                      st, 0, i, 0, NULL, FLAG_NULL, compoundmiddle,
                      hu_mov_rule ? IN_CPD_OTHER : IN_CPD_BEGIN)) ||
                 (compoundmoresuffixes &&
                  (entry_result = suffix_check_twosfx(
                       st, 0, i, 0, NULL,
                       compoundmiddle))) ||  // twofold suffixes + compound
                 (entry_result = prefix_check(st, 0, i,
                                    hu_mov_rule ? IN_CPD_OTHER : IN_CPD_BEGIN,
                                    compoundmiddle))))))
            checked_prefix = 1;
          // else check forbiddenwords and needaffix
        } else if (entry_result->astr && (TESTAFF(entry_result->astr, forbiddenword, entry_result->alen) ||
                                TESTAFF(entry_result->astr, needaffix, entry_result->alen) ||
                                TESTAFF(entry_result->astr, ONLYUPCASEFLAG, entry_result->alen) ||
                                (is_sug && nosuggest &&
                                 TESTAFF(entry_result->astr, nosuggest, entry_result->alen)))) {
          st[i] = ch;
          // continue;
          break;
        }

        // check non_compound flag in suffix and prefix
        if ((entry_result) && !hu_mov_rule &&
            ((pfx && pfx->getCont() &&
              TESTAFF(pfx->getCont(), compoundforbidflag, pfx->getContLen())) ||
             (sfx && sfx->getCont() &&
              TESTAFF(sfx->getCont(), compoundforbidflag,
                      sfx->getContLen())))) {
          entry_result = NULL;
        }

        // check compoundend flag in suffix and prefix
        if ((entry_result) && !checked_prefix && compoundend && !hu_mov_rule &&
            ((pfx && pfx->getCont() &&
              TESTAFF(pfx->getCont(), compoundend, pfx->getContLen())) ||
             (sfx && sfx->getCont() &&
              TESTAFF(sfx->getCont(), compoundend, sfx->getContLen())))) {
          entry_result = NULL;
        }

        // check compoundmiddle flag in suffix and prefix
        if ((entry_result) && !checked_prefix && (wordnum == 0) && compoundmiddle &&
            !hu_mov_rule &&
            ((pfx && pfx->getCont() &&
              TESTAFF(pfx->getCont(), compoundmiddle, pfx->getContLen())) ||
             (sfx && sfx->getCont() &&
              TESTAFF(sfx->getCont(), compoundmiddle, sfx->getContLen())))) {
          entry_result = NULL;
        }

        // check forbiddenwords
        if ((entry_result) && (entry_result->astr) &&
            (TESTAFF(entry_result->astr, forbiddenword, entry_result->alen) ||
             TESTAFF(entry_result->astr, ONLYUPCASEFLAG, entry_result->alen) ||
             (is_sug && nosuggest && TESTAFF(entry_result->astr, nosuggest, entry_result->alen)))) {
          return NULL;
        }

        // increment word number, if the second root has a compoundroot flag
        if ((entry_result) && compoundroot &&
            (TESTAFF(entry_result->astr, compoundroot, entry_result->alen))) {
          wordnum++;
        }

        // first word is acceptable in compound words?
        if (((entry_result) &&
             (checked_prefix || (words && words[wnum]) ||
              (compoundflag && TESTAFF(entry_result->astr, compoundflag, entry_result->alen)) ||
              ((oldwordnum == 0) && compoundbegin &&
               TESTAFF(entry_result->astr, compoundbegin, entry_result->alen)) ||
              ((oldwordnum > 0) && compoundmiddle &&
               TESTAFF(entry_result->astr, compoundmiddle, entry_result->alen))

              // LANG_hu section: spec. Hungarian rule
              || ((langnum == LANG_hu) && hu_mov_rule &&
                  (TESTAFF(
                       entry_result->astr, 'F',
                       entry_result->alen) ||  // XXX hardwired Hungarian dictionary codes
                   TESTAFF(entry_result->astr, 'G', entry_result->alen) ||
                   TESTAFF(entry_result->astr, 'H', entry_result->alen)))
              // END of LANG_hu section
              ) &&
             (
                 // test CHECKCOMPOUNDPATTERN conditions
                 scpd == 0 || checkcpdtable[scpd - 1].cond == FLAG_NULL ||
                 TESTAFF(entry_result->astr, checkcpdtable[scpd - 1].cond, entry_result->alen)) &&
             !((checkcompoundtriple && scpd == 0 &&
                !words && i < word.size() && // test triple letters
                (word[i - 1] == word[i]) &&
                (((i > 1) && (word[i - 1] == word[i - 2])) ||
                 ((word[i - 1] == word[i + 1]))  // may be word[i+1] == '\0'
                 )) ||
               (checkcompoundcase && scpd == 0 && !words && i < word.size() &&
                cpdcase_check(word, i))))
            // LANG_hu section: spec. Hungarian rule
            || ((!entry_result) && (langnum == LANG_hu) && hu_mov_rule &&
                (entry_result = affix_check(st, 0, i)) &&
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
          rv_first = entry_result;
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

            entry_result = lookup(st.c_str() + i, st.size() - i);  // perhaps without prefix

            // search homonym with compound flag
            while ((entry_result) &&
                   ((needaffix && TESTAFF(entry_result->astr, needaffix, entry_result->alen)) ||
                    !((compoundflag && !words &&
                       TESTAFF(entry_result->astr, compoundflag, entry_result->alen)) ||
                      (compoundend && !words &&
                       TESTAFF(entry_result->astr, compoundend, entry_result->alen)) ||
                      (!defcpdtable.empty() && words &&
                       defcpd_check(&words, wnum + 1, entry_result, NULL, 1))) ||
                    (scpd != 0 && checkcpdtable[scpd - 1].cond2 != FLAG_NULL &&
                     !TESTAFF(entry_result->astr, checkcpdtable[scpd - 1].cond2,
                              entry_result->alen)))) {
              entry_result = entry_result->next_homonym;
            }

            // check FORCEUCASE
            if (entry_result && forceucase &&
                (TESTAFF(entry_result->astr, forceucase, entry_result->alen)) &&
                !(info && *info & SPELL_ORIGCAP))
              entry_result = NULL;

            if (entry_result && words && words[wnum + 1])
              return rv_first;

            oldnumsyllable2 = numsyllable;
            oldwordnum2 = wordnum;

            // LANG_hu section: spec. Hungarian rule, XXX hardwired dictionary
            // code
            if ((entry_result) && (langnum == LANG_hu) &&
                (TESTAFF(entry_result->astr, 'I', entry_result->alen)) &&
                !(TESTAFF(entry_result->astr, 'J', entry_result->alen))) {
              numsyllable--;
            }
            // END of LANG_hu section

            // increment word number, if the second root has a compoundroot flag
            if ((entry_result) && (compoundroot) &&
                (TESTAFF(entry_result->astr, compoundroot, entry_result->alen))) {
              wordnum++;
            }

            // check forbiddenwords
            if ((entry_result) && (entry_result->astr) &&
                (TESTAFF(entry_result->astr, forbiddenword, entry_result->alen) ||
                 TESTAFF(entry_result->astr, ONLYUPCASEFLAG, entry_result->alen) ||
                 (is_sug && nosuggest &&
                  TESTAFF(entry_result->astr, nosuggest, entry_result->alen))))
              return NULL;

            // second word is acceptable, as a root?
            // hungarian conventions: compounding is acceptable,
            // when compound forms consist of 2 words, or if more,
            // then the syllable number of root words must be 6, or lesser.

            if ((entry_result) &&
                ((compoundflag && TESTAFF(entry_result->astr, compoundflag, entry_result->alen)) ||
                 (compoundend && TESTAFF(entry_result->astr, compoundend, entry_result->alen))) &&
                (((cpdwordmax == -1) || (wordnum + 1 < cpdwordmax)) ||
                 ((cpdmaxsyllable != 0) &&
                  (numsyllable + get_syllable(std::string(HENTRY_WORD(entry_result), entry_result->blen)) <=
                   cpdmaxsyllable))) &&
                (
                    // test CHECKCOMPOUNDPATTERN
                    checkcpdtable.empty() || scpd != 0 ||
                    (i < word.size() && !cpdpat_check(word, i, rv_first, entry_result, 0))) &&
                ((!checkcompounddup || (entry_result != rv_first)))
                // test CHECKCOMPOUNDPATTERN conditions
                &&
                (scpd == 0 || checkcpdtable[scpd - 1].cond2 == FLAG_NULL ||
                 TESTAFF(entry_result->astr, checkcpdtable[scpd - 1].cond2, entry_result->alen))) {
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
            entry_result = (compoundflag && !onlycpdrule && i < word.size())
                     ? affix_check(word, i, word.size() - i, compoundflag,
                                   IN_CPD_END)
                     : NULL;
            if (!entry_result && compoundend && !onlycpdrule) {
              sfx = NULL;
              pfx = NULL;
              if (i < word.size())
                entry_result = affix_check(word, i, word.size() - i, compoundend, IN_CPD_END);
            }

            if (!entry_result && !defcpdtable.empty() && words) {
              if (i < word.size())
                entry_result = affix_check(word, i, word.size() - i, 0, IN_CPD_END);
              if (entry_result && defcpd_check(&words, wnum + 1, entry_result, NULL, 1))
                return rv_first;
              entry_result = NULL;
            }

            // test CHECKCOMPOUNDPATTERN conditions (allowed forms)
            if (entry_result &&
                !(scpd == 0 || checkcpdtable[scpd - 1].cond2 == FLAG_NULL ||
                  TESTAFF(entry_result->astr, checkcpdtable[scpd - 1].cond2, entry_result->alen)))
              entry_result = NULL;

            // test CHECKCOMPOUNDPATTERN conditions (forbidden compounds)
            if (entry_result && !checkcpdtable.empty() && scpd == 0 &&
                cpdpat_check(word, i, rv_first, entry_result, affixed))
              entry_result = NULL;

            // check non_compound flag in suffix and prefix
            if ((entry_result) && ((pfx && pfx->getCont() &&
                          TESTAFF(pfx->getCont(), compoundforbidflag,
                                  pfx->getContLen())) ||
                         (sfx && sfx->getCont() &&
                          TESTAFF(sfx->getCont(), compoundforbidflag,
                                  sfx->getContLen())))) {
              entry_result = NULL;
            }

            // check FORCEUCASE
            if (entry_result && forceucase &&
                (TESTAFF(entry_result->astr, forceucase, entry_result->alen)) &&
                !(info && *info & SPELL_ORIGCAP))
              entry_result = NULL;

            // check forbiddenwords
            if ((entry_result) && (entry_result->astr) &&
                (TESTAFF(entry_result->astr, forbiddenword, entry_result->alen) ||
                 TESTAFF(entry_result->astr, ONLYUPCASEFLAG, entry_result->alen) ||
                 (is_sug && nosuggest &&
                  TESTAFF(entry_result->astr, nosuggest, entry_result->alen))))
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
                    if (entry_result && TESTAFF(entry_result->astr, 'J', entry_result->alen))
                      numsyllable += 1;
                    break;
                  }
                }
              }
            }

            // increment word number, if the second word has a compoundroot flag
            if ((entry_result) && (compoundroot) &&
                (TESTAFF(entry_result->astr, compoundroot, entry_result->alen))) {
              wordnum++;
            }
            // second word is acceptable, as a word with prefix or/and suffix?
            // hungarian conventions: compounding is acceptable,
            // when compound forms consist 2 word, otherwise
            // the syllable number of root words is 6, or lesser.
            if ((entry_result) &&
                (((cpdwordmax == -1) || (wordnum + 1 < cpdwordmax)) ||
                 ((cpdmaxsyllable != 0) && (numsyllable <= cpdmaxsyllable))) &&
                ((!checkcompounddup || (entry_result != rv_first)))) {
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
            // (only if SPELL_COMPOUND_2 is not set and maxwordnum is not exceeded)
            if ((!info || !(*info & SPELL_COMPOUND_2)) && wordnum + 2 < maxwordnum) {
              entry_result = compound_check(st.substr(i), wordnum + 1,
                                  numsyllable, maxwordnum, wnum + 1, words, rwords, 0,
                                  is_sug, info);

              if (entry_result && !checkcpdtable.empty() && i < word.size() &&
                  ((scpd == 0 &&
                    cpdpat_check(word, i, rv_first, entry_result, affixed)) ||
                   (scpd != 0 &&
                    !cpdpat_check(word, i, rv_first, entry_result, affixed))))
                entry_result = NULL;
            } else {
              entry_result = NULL;
            }
            if (entry_result) {
              // forbid compound word, if it is a non-compound word with typical
              // fault, or a dictionary word pair

              if (cpdwordpair_check(word, len))
                  return NULL;

              if (checkcompoundrep || forbiddenword) {

                if (checkcompoundrep && cpdrep_check(word, len))
                  return NULL;

                // check first part
                if (i < word.size() && word.compare(i, entry_result->blen, entry_result->word, entry_result->blen) == 0) {
                  char r = st[i + entry_result->blen];
                  st[i + entry_result->blen] = '\0';

                  if ((checkcompoundrep && cpdrep_check(st, i + entry_result->blen)) ||
                      cpdwordpair_check(st, i + entry_result->blen)) {
                    st[ + i + entry_result->blen] = r;
                    continue;
                  }

                  if (forbiddenword) {
                    struct hentry* rv2 = lookup(word.c_str(), word.size());
                    if (!rv2 && len <= word.size())
                      rv2 = affix_check(word, 0, len);
                    if (rv2 && rv2->astr &&
                        TESTAFF(rv2->astr, forbiddenword, rv2->alen) &&
                        (strncmp(rv2->word, st.c_str(), i + entry_result->blen) == 0)) {
                      return NULL;
                    }
                  }
                  st[i + entry_result->blen] = r;
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