
            for (i = 0; i < FF_ARRAY_ELEMS(cand_lists); i++) {
                RefPicList *rps = &s->rps[cand_lists[i]];
                for (j = 0; j < rps->nb_refs && rpl_tmp.nb_refs < HEVC_MAX_REFS; j++) {
                    rpl_tmp.list[rpl_tmp.nb_refs]       = rps->list[j];
                    rpl_tmp.ref[rpl_tmp.nb_refs]        = rps->ref[j];
                    rpl_tmp.isLongTerm[rpl_tmp.nb_refs] = i == 2;
                    rpl_tmp.nb_refs++;
                }
            }
            // Construct RefPicList0, RefPicList1 (8-8, 8-10)
            if (s->ps.pps->pps_curr_pic_ref_enabled_flag && rpl_tmp.nb_refs < HEVC_MAX_REFS) {
                rpl_tmp.list[rpl_tmp.nb_refs]           = s->ref->poc;
                rpl_tmp.ref[rpl_tmp.nb_refs]            = s->ref;
                rpl_tmp.isLongTerm[rpl_tmp.nb_refs]     = 1;
                rpl_tmp.nb_refs++;
            }
        