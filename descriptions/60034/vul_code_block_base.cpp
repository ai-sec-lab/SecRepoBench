// forbid dictionary stems with COMPOUNDFORBIDFLAG in
        // compound words, overriding the effect of COMPOUNDPERMITFLAG
        if ((rv) && compoundforbidflag &&
                TESTAFF(rv->astr, compoundforbidflag, rv->alen) && !hu_mov_rule) {
            // given the while conditions that continue jumps to, this situation
            // never ends
            if (!scpd && !onlycpdrule && simplifiedcpd) {
                HUNSPELL_WARNING(stderr, "break infinite loop\n");
                break;
            }
            continue;
        }

        // search homonym with compound flag
        while ((rv) && !hu_mov_rule &&
               ((needaffix && TESTAFF(rv->astr, needaffix, rv->alen)) ||
                !((compoundflag && !words && !onlycpdrule &&
                   TESTAFF(rv->astr, compoundflag, rv->alen)) ||
                  (compoundbegin && !wordnum && !onlycpdrule &&
                   TESTAFF(rv->astr, compoundbegin, rv->alen)) ||
                  (compoundmiddle && wordnum && !words && !onlycpdrule &&
                   TESTAFF(rv->astr, compoundmiddle, rv->alen)) ||
                  (!defcpdtable.empty() && onlycpdrule &&
                   ((!words && !wordnum &&
                     defcpd_check(&words, wnum, rv, rwords, 0)) ||
                    (words &&
                     defcpd_check(&words, wnum, rv, rwords, 0))))) ||
                (scpd != 0 && checkcpdtable[scpd - 1].cond != FLAG_NULL &&
                 !TESTAFF(rv->astr, checkcpdtable[scpd - 1].cond, rv->alen)))) {
          rv = rv->next_homonym;
        }