/* 64 */
        struct fat_arch_64 * fa = 0;
        fa = (struct fat_arch_64 *)calloc(duhd.au_count,
            sizeof(struct fat_arch_64));
        if (!fa) {
            *errcode = DW_DLE_ALLOC_FAIL;
            free(duhd.au_arches);
            duhd.au_arches = 0;
            return DW_DLV_ERROR;
        }