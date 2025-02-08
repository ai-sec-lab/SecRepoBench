int ff_hevc_slice_rpl(HEVCContext *s)
{
    SliceHeader *sh = &s->sh;

    uint8_t nb_list = sh->slice_type == HEVC_SLICE_B ? 2 : 1;
    uint8_t listindex;
    int i, j, ret;

    ret = init_slice_rpl(s);
    if (ret < 0)
        return ret;

    if (!(s->rps[ST_CURR_BEF].nb_refs + s->rps[ST_CURR_AFT].nb_refs +
          s->rps[LT_CURR].nb_refs) && !s->ps.pps->pps_curr_pic_ref_enabled_flag) {
        av_log(s->avctx, AV_LOG_ERROR, "Zero refs in the frame RPS.\n");
        return AVERROR_INVALIDDATA;
    }

    for (listindex = 0; listindex < nb_list; listindex++) {
        RefPicList  rpl_tmp = { { 0 } };
        RefPicList *rpl     = &s->ref->refPicList[listindex];

        /* The order of the elements is
         * ST_CURR_BEF - ST_CURR_AFT - LT_CURR for the L0 and
         * ST_CURR_AFT - ST_CURR_BEF - LT_CURR for the L1 */
        int cand_lists[3] = { listindex ? ST_CURR_AFT : ST_CURR_BEF,
                              listindex ? ST_CURR_BEF : ST_CURR_AFT,
                              LT_CURR };

        /* concatenate the candidate lists for the current frame */
        while (rpl_tmp.nb_refs < sh->nb_refs[listindex]) {
            // <MASK>
        }

        /* reorder the references if necessary */
        if (sh->rpl_modification_flag[listindex]) {
            for (i = 0; i < sh->nb_refs[listindex]; i++) {
                int idx = sh->list_entry_lx[listindex][i];

                if (idx >= rpl_tmp.nb_refs) {
                    av_log(s->avctx, AV_LOG_ERROR, "Invalid reference index.\n");
                    return AVERROR_INVALIDDATA;
                }

                rpl->list[i]       = rpl_tmp.list[idx];
                rpl->ref[i]        = rpl_tmp.ref[idx];
                rpl->isLongTerm[i] = rpl_tmp.isLongTerm[idx];
                rpl->nb_refs++;
            }
        } else {
            memcpy(rpl, &rpl_tmp, sizeof(*rpl));
            rpl->nb_refs = FFMIN(rpl->nb_refs, sh->nb_refs[listindex]);
        }

        // 8-9
        if (s->ps.pps->pps_curr_pic_ref_enabled_flag &&
            !sh->rpl_modification_flag[listindex] &&
            rpl_tmp.nb_refs > sh->nb_refs[L0]) {
            rpl->list[sh->nb_refs[L0] - 1] = s->ref->poc;
            rpl->ref[sh->nb_refs[L0] - 1]  = s->ref;
        }

        if (sh->collocated_list == listindex &&
            sh->collocated_ref_idx < rpl->nb_refs)
            s->ref->collocated_ref = rpl->ref[sh->collocated_ref_idx];
    }

    return 0;
}