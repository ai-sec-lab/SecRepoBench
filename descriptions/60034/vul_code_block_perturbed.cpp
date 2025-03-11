// forbid dictionary stems with COMPOUNDFORBIDFLAG in
        // compound words, overriding the effect of COMPOUNDPERMITFLAG
        if ((entry_result) && compoundforbidflag &&
                TESTAFF(entry_result->astr, compoundforbidflag, entry_result->alen) && !hu_mov_rule) {
            // given the while conditions that continue jumps to, this situation
            // never ends
            if (!scpd && !onlycpdrule && simplifiedcpd) {
                HUNSPELL_WARNING(stderr, "break infinite loop\n");
                break;
            }
            continue;
        }

        // search homonym with compound flag
        while ((entry_result) && !hu_mov_rule &&
               ((needaffix && TESTAFF(entry_result->astr, needaffix, entry_result->alen)) ||
                !((compoundflag && !words && !onlycpdrule &&
                   TESTAFF(entry_result->astr, compoundflag, entry_result->alen)) ||
                  (compoundbegin && !wordnum && !onlycpdrule &&
                   TESTAFF(entry_result->astr, compoundbegin, entry_result->alen)) ||
                  (compoundmiddle && wordnum && !words && !onlycpdrule &&
                   TESTAFF(entry_result->astr, compoundmiddle, entry_result->alen)) ||
                  (!defcpdtable.empty() && onlycpdrule &&
                   ((!words && !wordnum &&
                     defcpd_check(&words, wnum, entry_result, rwords, 0)) ||
                    (words &&
                     defcpd_check(&words, wnum, entry_result, rwords, 0))))) ||
                (scpd != 0 && checkcpdtable[scpd - 1].cond != FLAG_NULL &&
                 !TESTAFF(entry_result->astr, checkcpdtable[scpd - 1].cond, entry_result->alen)))) {
          entry_result = entry_result->next_homonym;
        }