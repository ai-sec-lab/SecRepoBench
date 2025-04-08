static int add_candidate_ref(HEVCContext *s, RefPicList *list,
                             int poc, int ref_flag)
{
    // Attempt to find a reference frame in the DPB with a given POC (picture order count).
    // If the found reference is the same as the current reference, 
    // return an error to prevent adding a duplicate or overflow.
    // If no reference is found, attempt to generate a missing reference frame for the given POC.
    // <MASK>

    list->list[list->nb_refs] = ref->poc;
    list->ref[list->nb_refs]  = ref;
    list->nb_refs++;

    mark_ref(ref, ref_flag);
    return 0;
}