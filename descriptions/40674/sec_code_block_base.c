if (count >= ep->f_loc_shdr.g_count) {
            /* Impossible */
            free(data);
            *errcode = DW_DLE_ELF_SECTION_GROUP_ERROR;
            return DW_DLV_ERROR;
        }
        res = RRMOA(ep->f_fd,data,psh->gh_offset,seclen,
            ep->f_filesize,errcode);
        if (res != DW_DLV_OK) {
            free(data);
            return res;
        }
        grouparray = malloc(count * sizeof(Dwarf_Unsigned));
        if (!grouparray) {
            free(data);
            *errcode = DW_DLE_ALLOC_FAIL;
            return DW_DLV_ERROR;
        }

        memcpy(dblock,dp,DWARF_32BIT_SIZE);
        ASNAR(memcpy,va,dblock);
        /* There is ambiguity on the endianness of this stuff. */
        if (va != 1 && va != 0x1000000) {
            /*  Could be corrupted elf object. */
            *errcode = DW_DLE_ELF_SECTION_GROUP_ERROR;
            free(data);
            free(grouparray);
            return DW_DLV_ERROR;
        }
        grouparray[0] = 1;
        dp = dp + DWARF_32BIT_SIZE;
        for ( i = 1; i < count; ++i,dp += DWARF_32BIT_SIZE) {
            Dwarf_Unsigned gseca = 0;
            Dwarf_Unsigned gsecb = 0;
            struct generic_shdr* targpsh = 0;

            memcpy(dblock,dp,DWARF_32BIT_SIZE);
            ASNAR(memcpy,gseca,dblock);
            ASNAR(_dwarf_memcpy_swap_bytes,gsecb,dblock);
            if (!gseca) {
                free(data);
                free(grouparray);
                *errcode = DW_DLE_ELF_SECTION_GROUP_ERROR;
                return DW_DLV_ERROR;
            }
            grouparray[i] = gseca;
            if (gseca >= ep->f_loc_shdr.g_count) {
                /*  Might be confused endianness by
                    the compiler generating the SHT_GROUP.
                    This is pretty horrible. */

                if (gsecb >= ep->f_loc_shdr.g_count) {
                    *errcode = DW_DLE_ELF_SECTION_GROUP_ERROR;
                    free(data);
                    free(grouparray);
                    return DW_DLV_ERROR;
                }
                /* Ok. Yes, ugly. */
                gseca = gsecb;
                grouparray[i] = gseca;
            }
            targpsh = ep->f_shdr + gseca;
            if (targpsh->gh_section_group_number) {
                /* multi-assignment to groups. Oops. */
                free(data);
                free(grouparray);
                *errcode = DW_DLE_ELF_SECTION_GROUP_ERROR;
                return DW_DLV_ERROR;
            }
            targpsh->gh_section_group_number =
                ep->f_sg_next_group_number;
            foundone = 1;
        }