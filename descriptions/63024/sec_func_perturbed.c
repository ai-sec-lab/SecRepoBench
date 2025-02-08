static int
_dwarf_object_detector_universal_head_fd(
    int fd,
    Dwarf_Unsigned      dw_filesize,
    unsigned int      *dw_contentcount,
    Dwarf_Universal_Head * dw_head,
    int                *errcode)
{
    struct Dwarf_Universal_Head_s  duhead;
    struct Dwarf_Universal_Head_s *duhdp = 0;
    struct  fat_header fh;
    int     res = 0;
    void (*word_swap) (void *, const void *, unsigned long);
    int     locendian = 0;
    int     locoffsetsize = 0;

    duhead = duhzero;
    fh = fhzero;
    /*  A universal head is always at offset zero. */
    duhead.au_filesize = dw_filesize;
    if (sizeof(fh) >= dw_filesize) {
        *errcode = DW_DLE_UNIVERSAL_BINARY_ERROR;
        return DW_DLV_ERROR;
    }
    res = RRMOA(fd,&fh,0,sizeof(fh), dw_filesize,errcode);
    if (res != DW_DLV_OK) {
        return res;
    }
    duhead.au_magic = magic_copy((unsigned char *)&fh.magic[0],4);
    if (duhead.au_magic == FAT_MAGIC) {
        locendian = DW_END_big;
        locoffsetsize = 32;
    } else if (duhead.au_magic == FAT_CIGAM) {
        locendian = DW_END_little;
        locoffsetsize = 32;
    }else if (duhead.au_magic == FAT_MAGIC_64) {
        locendian = DW_END_big;
        locoffsetsize = 64;
    } else if (duhead.au_magic == FAT_CIGAM_64) {
        locendian = DW_END_little;
        locoffsetsize = 64;
    } else {
        *errcode = DW_DLE_FILE_WRONG_TYPE;
        return DW_DLV_ERROR;
    }
#ifdef WORDS_BIGENDIAN
    if (locendian == DW_END_little) {
        word_swap = _dwarf_memcpy_swap_bytes;
    } else {
        word_swap = _dwarf_memcpy_noswap_bytes;
    }
#else  /* LITTLE ENDIAN */
    if (locendian == DW_END_little) {
        word_swap = _dwarf_memcpy_noswap_bytes;
    } else {
        word_swap = _dwarf_memcpy_swap_bytes;
    }
#endif /* LITTLE- BIG-ENDIAN */
    ASNAR(word_swap,duhead.au_count,fh.nfat_arch);
    /*  The limit is a first-cut safe heuristic. */
    if (duhead.au_count >= (dw_filesize/2) ) {
        *errcode = DW_DLE_UNIVERSAL_BINARY_ERROR ;
        return DW_DLV_ERROR;
    }
    duhead.au_arches = (struct  Dwarf_Universal_Arch_s*)
        calloc(duhead.au_count, sizeof(struct Dwarf_Universal_Arch_s));
    if (!duhead.au_arches) {
        *errcode = DW_DLE_ALLOC_FAIL;
        return DW_DLV_ERROR;
    }
    if (locoffsetsize == 32) {
        struct fat_arch * fa = 0;
        fa = (struct fat_arch *)calloc(duhead.au_count,
            sizeof(struct fat_arch));
        if (!fa) {
            *errcode = DW_DLE_ALLOC_FAIL;
            free(duhead.au_arches);
            duhead.au_arches = 0;
            free(fa);
            return DW_DLV_ERROR;
        }
        if (sizeof(fh)+duhead.au_count*sizeof(*fa) >= dw_filesize) {
            free(duhead.au_arches);
            duhead.au_arches = 0;
            free(fa);
            *errcode = DW_DLE_FILE_OFFSET_BAD;
            return DW_DLV_ERROR;
        }
        res = RRMOA(fd,fa,/*offset=*/sizeof(fh),
            duhead.au_count*sizeof(*fa),
            dw_filesize,errcode);
        if (res != DW_DLV_OK) {
            free(duhead.au_arches);
            duhead.au_arches = 0;
            free(fa);
            return res;
        }
        res = fill_in_uni_arch_32(fa,&duhead,word_swap,
            errcode);
        free(fa);
        fa = 0;
        if (res != DW_DLV_OK) {
            free(duhead.au_arches);
            duhead.au_arches = 0;
            return res;
        }
    } else { /* 64 */
        struct fat_arch_64 * fa = 0;
        fa = (struct fat_arch_64 *)calloc(duhead.au_count,
            sizeof(struct fat_arch_64));
        if (!fa) {
            *errcode = DW_DLE_ALLOC_FAIL;
            free(duhead.au_arches);
            duhead.au_arches = 0;
            return DW_DLV_ERROR;
        }
        if (sizeof(fh)+duhead.au_count*sizeof(*fa) >= dw_filesize) {
            free(duhead.au_arches);
            duhead.au_arches = 0;
            free(fa);
            *errcode = DW_DLE_FILE_OFFSET_BAD ;
            return DW_DLV_ERROR;
        }
        res = RRMOA(fd,fa,/*offset*/sizeof(fh),
            duhead.au_count*sizeof(fa),
            dw_filesize,errcode);
        if (res == DW_DLV_ERROR) {
            free(duhead.au_arches);
            duhead.au_arches = 0;
            free(fa);
            return res;
        }
        res = fill_in_uni_arch_64(fa,&duhead,word_swap,
            errcode);
        free(fa);
        fa = 0;
        if (res != DW_DLV_OK) {
            free(duhead.au_arches);
            duhead.au_arches = 0;
            return res;
        }
    }

    duhdp = malloc(sizeof(*duhdp));
    if (!duhdp) {
        free(duhead.au_arches);
        duhead.au_arches = 0;
        *errcode = DW_DLE_ALLOC_FAIL;
        return res;
    }
    memcpy(duhdp,&duhead,sizeof(duhead));
    *dw_contentcount = duhead.au_count;
    duhdp->au_arches = duhead.au_arches;
    *dw_head = duhdp;
    return DW_DLV_OK;
}