/* 64 */
        struct fat_arch_64 * fa = 0;
        fa = (struct fat_arch_64 *)calloc(duhead.au_count,
            sizeof(struct fat_arch));
        if (!fa) {
            *errcode = DW_DLE_ALLOC_FAIL;
            free(duhead.au_arches);
            duhead.au_arches = 0;
            return DW_DLV_ERROR;
        }