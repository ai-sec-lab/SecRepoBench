(rv) &&
                ((compoundflag && TESTAFF(rv->astr, compoundflag, rv->alen)) ||
                 (compoundend && TESTAFF(rv->astr, compoundend, rv->alen))) &&
                (((cpdwordmax == -1) || (wordnum + 1 < cpdwordmax)) ||
                 ((cpdmaxsyllable != 0) &&
                  (numsyllable + get_syllable(std::string(HENTRY_WORD(rv), rv->blen)) <=
                   cpdmaxsyllable))) &&
                (
                    // test CHECKCOMPOUNDPATTERN
                    checkcpdtable.empty() || scpd != 0 ||
                    (i < word.size() && !cpdpat_check(word, i, rv_first, rv, 0))) &&
                ((!checkcompounddup || (rv != rv_first)))
                // test CHECKCOMPOUNDPATTERN conditions
                &&
                (scpd == 0 || checkcpdtable[scpd - 1].cond2 == FLAG_NULL ||
                 TESTAFF(rv->astr, checkcpdtable[scpd - 1].cond2, rv->alen))