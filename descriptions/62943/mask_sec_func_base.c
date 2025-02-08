static int
_dwarf_object_detector_universal_head_fd(
    int fd,
    Dwarf_Unsigned      dw_filesize,
    unsigned int      *dw_contentcount,
    Dwarf_Universal_Head * dw_head,
    int                *errcode)
{
    struct Dwarf_Universal_Head_s  duhd;
    struct Dwarf_Universal_Head_s *duhdp = 0;
    struct  fat_header fh;
    int     res = 0;
    void (*word_swap) (void *, const void *, unsigned long);
    int     locendian = 0;
    int     locoffsetsize = 0;

    duhd = duhzero;
    fh = fhzero;
    /*  A universal head is always at offset zero. */
    // <MASK>

    duhdp = malloc(sizeof(*duhdp));
    if (!duhdp) {
        free(duhd.au_arches);
        duhd.au_arches = 0;
        *errcode = DW_DLE_ALLOC_FAIL;
        return res;
    }
    memcpy(duhdp,&duhd,sizeof(duhd));
    *dw_contentcount = duhd.au_count;
    duhdp->au_arches = duhd.au_arches;
    *dw_head = duhdp;
    return DW_DLV_OK;
}