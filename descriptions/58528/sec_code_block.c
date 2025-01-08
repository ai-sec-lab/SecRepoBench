if (s->ps.pps->pps_curr_pic_ref_enabled_flag && rpl_tmp.nb_refs < HEVC_MAX_REFS) {
                rpl_tmp.list[rpl_tmp.nb_refs]           = s->ref->poc;
                rpl_tmp.ref[rpl_tmp.nb_refs]            = s->ref;
                rpl_tmp.isLongTerm[rpl_tmp.nb_refs]     = 1;
                rpl_tmp.nb_refs++;
            }