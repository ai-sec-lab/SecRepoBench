static int add_candidate_ref(HEVCContext *s, RefPicList *list,
                             int pictureordercount, int ref_flag)
{
    HEVCFrame *ref = find_ref_idx(s, pictureordercount);

    if (ref == s->ref || list->nb_refs >= HEVC_MAX_REFS)
        return AVERROR_INVALIDDATA;

    if (!ref) {
        ref = generate_missing_ref(s, pictureordercount);
        if (!ref)
            return AVERROR(ENOMEM);
    }

    list->list[list->nb_refs] = ref->poc;
    list->ref[list->nb_refs]  = ref;
    list->nb_refs++;

    mark_ref(ref, ref_flag);
    return 0;
}