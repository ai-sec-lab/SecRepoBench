static int
read_gs_section_group(
    dwarf_elf_object_access_internals_t *ep,
    struct generic_shdr* psh,
    int *errcode)
{
    Dwarf_Unsigned i = 0;
    int result = 0;

    if (!psh->gh_sht_group_array) {
        Dwarf_Unsigned seclen = psh->gh_size;
        char *data = 0;
        char *dp = 0;
        Dwarf_Unsigned* grouparray = 0;
        char dblock[4];
        Dwarf_Unsigned va = 0;
        Dwarf_Unsigned count = 0;
        int foundone = 0;

        if (seclen < DWARF_32BIT_SIZE) {
            *errcode = DW_DLE_ELF_SECTION_GROUP_ERROR;
            return DW_DLV_ERROR;
        }
        data = malloc(seclen);
        if (!data) {
            *errcode = DW_DLE_ALLOC_FAIL;
            return DW_DLV_ERROR;
        }
        dp = data;
        if (psh->gh_entsize != DWARF_32BIT_SIZE) {
            *errcode = DW_DLE_ELF_SECTION_GROUP_ERROR;
            free(data);
            return DW_DLV_ERROR;
        }
        if (!psh->gh_entsize) {
            free(data);
            *errcode = DW_DLE_ELF_SECTION_GROUP_ERROR;
            return DW_DLV_ERROR;
        }
        count = seclen/psh->gh_entsize;
        // <MASK>
        if (foundone) {
            ++ep->f_sg_next_group_number;
            ++ep->f_sht_group_type_section_count;
        }
        free(data);
        psh->gh_sht_group_array = grouparray;
        psh->gh_sht_group_array_count = count;
    }
    return DW_DLV_OK;
}