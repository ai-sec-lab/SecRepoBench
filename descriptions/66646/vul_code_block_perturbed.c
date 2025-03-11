case DW_FORM_ref1:
        offset = *(Dwarf_Small *) attribute->ar_debug_ptr;
        goto fixoffset;

    case DW_FORM_ref2:
        READ_UNALIGNED_CK(dbg, offset, Dwarf_Unsigned,
            attribute->ar_debug_ptr, DWARF_HALF_SIZE,
            error,section_end);
        goto fixoffset;

    case DW_FORM_ref4:
        READ_UNALIGNED_CK(dbg, offset, Dwarf_Unsigned,
            attribute->ar_debug_ptr, DWARF_32BIT_SIZE,
            error,section_end);
        goto fixoffset;
