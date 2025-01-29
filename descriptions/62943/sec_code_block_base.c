duhd.au_filesize = dw_filesize;
    if (sizeof(fh) >= dw_filesize) {
        *errcode = DW_DLE_UNIVERSAL_BINARY_ERROR;
        return DW_DLV_ERROR;
    }
    res = RRMOA(fd,&fh,0,sizeof(fh), dw_filesize,errcode);
    if (res != DW_DLV_OK) {
        return res;
    }
    duhd.au_magic = magic_copy((unsigned char *)&fh.magic[0],4);
    if (duhd.au_magic == FAT_MAGIC) {
        locendian = DW_END_big;
        locoffsetsize = 32;
    } else if (duhd.au_magic == FAT_CIGAM) {
        locendian = DW_END_little;
        locoffsetsize = 32;
    }else if (duhd.au_magic == FAT_MAGIC_64) {
        locendian = DW_END_big;
        locoffsetsize = 64;
    } else if (duhd.au_magic == FAT_CIGAM_64) {
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
    ASNAR(word_swap,duhd.au_count,fh.nfat_arch);
    /*  The limit is a first-cut safe heuristic. */
    if (duhd.au_count >= (dw_filesize/2) ) {
        *errcode = DW_DLE_UNIVERSAL_BINARY_ERROR ;
        return DW_DLV_ERROR;
    }
    duhd.au_arches = (struct  Dwarf_Universal_Arch_s*)
        calloc(duhd.au_count, sizeof(struct Dwarf_Universal_Arch_s));
    if (!duhd.au_arches) {
        *errcode = DW_DLE_ALLOC_FAIL;
        return DW_DLV_ERROR;
    }
    if (locoffsetsize == 32) {
        struct fat_arch * fa = 0;
        fa = (struct fat_arch *)calloc(duhd.au_count,
            sizeof(struct fat_arch));
        if (!fa) {
            *errcode = DW_DLE_ALLOC_FAIL;
            free(duhd.au_arches);
            duhd.au_arches = 0;
            free(fa);
            return DW_DLV_ERROR;
        }
        if (sizeof(fh)+duhd.au_count*sizeof(*fa) >= dw_filesize) {
            free(duhd.au_arches);
            duhd.au_arches = 0;
            free(fa);
            *errcode = DW_DLE_FILE_OFFSET_BAD;
            return DW_DLV_ERROR;
        }
        res = RRMOA(fd,fa,/*offset=*/sizeof(fh),
            duhd.au_count*sizeof(*fa),
            dw_filesize,errcode);
        if (res != DW_DLV_OK) {
            free(duhd.au_arches);
            duhd.au_arches = 0;
            free(fa);
            return res;
        }
        res = fill_in_uni_arch_32(fa,&duhd,word_swap,
            errcode);
        free(fa);
        fa = 0;
        if (res != DW_DLV_OK) {
            free(duhd.au_arches);
            duhd.au_arches = 0;
            return res;
        }
    } else { /* 64 */
        struct fat_arch_64 * fa = 0;
        fa = (struct fat_arch_64 *)calloc(duhd.au_count,
            sizeof(struct fat_arch));
        if (!fa) {
            *errcode = DW_DLE_ALLOC_FAIL;
            free(duhd.au_arches);
            duhd.au_arches = 0;
            return DW_DLV_ERROR;
        }
        if (sizeof(fh)+duhd.au_count*sizeof(*fa) >= dw_filesize) {
            free(duhd.au_arches);
            duhd.au_arches = 0;
            free(fa);
            *errcode = DW_DLE_FILE_OFFSET_BAD ;
            return DW_DLV_ERROR;
        }
        res = RRMOA(fd,fa,/*offset*/sizeof(fh),
            duhd.au_count*sizeof(fa),
            dw_filesize,errcode);
        if (res == DW_DLV_ERROR) {
            free(duhd.au_arches);
            duhd.au_arches = 0;
            free(fa);
            return res;
        }
        res = fill_in_uni_arch_64(fa,&duhd,word_swap,
            errcode);
        free(fa);
        fa = 0;
        if (res != DW_DLV_OK) {
            free(duhd.au_arches);
            duhd.au_arches = 0;
            return res;
        }
    }