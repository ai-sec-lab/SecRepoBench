// forbid dictionary stems with COMPOUNDFORBIDFLAG in
        // compound words, overriding the effect of COMPOUNDPERMITFLAG
        if ((rv) && compoundforbidflag &&
                TESTAFF(rv->astr, compoundforbidflag, rv->alen) && !hu_mov_rule) {
            bool would_continue = !onlycpdrule && simplifiedcpd;
            if (!scpd && would_continue) {
                // given the while conditions that continue jumps to, this situation
                // never ends
                HUNSPELL_WARNING(stderr, "break infinite loop\n");
                break;
            }

            if (scpd > 0 && would_continue) {
                // under these conditions we loop again, but the assumption above
                // appears to be that cmin and cmax are the original values they
                // had in the outside loop
                cmin = oldcmin;
                cmax = oldcmax;
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