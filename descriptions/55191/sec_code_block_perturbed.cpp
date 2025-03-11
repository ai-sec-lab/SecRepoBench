(rv) &&
             (checked_prefix || (words && words[wnum]) ||
              (compoundflag && TESTAFF(rv->astr, compoundflag, rv->alen)) ||
              ((oldwordnum == 0) && compoundbegin &&
               TESTAFF(rv->astr, compoundbegin, rv->alen)) ||
              ((oldwordnum > 0) && compoundmiddle &&
               TESTAFF(rv->astr, compoundmiddle, rv->alen))

              // LANG_hu section: spec. Hungarian rule
              || ((langnum == LANG_hu) && hu_mov_rule &&
                  (TESTAFF(
                       rv->astr, 'F',
                       rv->alen) ||  // XXX hardwired Hungarian dictionary codes
                   TESTAFF(rv->astr, 'G', rv->alen) ||
                   TESTAFF(rv->astr, 'H', rv->alen)))
              // END of LANG_hu section
              ) &&
             (
                 // test CHECKCOMPOUNDPATTERN conditions
                 scpd == 0 || checkcpdtable[scpd - 1].cond == FLAG_NULL ||
                 TESTAFF(rv->astr, checkcpdtable[scpd - 1].cond, rv->alen)) &&
             !((checkcompoundtriple && scpd == 0 &&
                !words && char_index < word.size() && // test triple letters
                (word[char_index - 1] == word[char_index]) &&
                (((char_index > 1) && (word[char_index - 1] == word[char_index - 2])) ||
                 ((word[char_index - 1] == word[char_index + 1]))  // may be word[i+1] == '\0'
                 )) ||
               (checkcompoundcase && scpd == 0 && !words && char_index < word.size() &&
                cpdcase_check(word, char_index)))