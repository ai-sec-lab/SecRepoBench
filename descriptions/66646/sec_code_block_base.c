case DW_FORM_ref1:
        if (attr->ar_debug_ptr >= section_end) {
            _dwarf_error_string(dbg, error, 
                DW_DLE_ATTR_FORM_OFFSET_BAD,
                "DW_DLE_ATTR_FORM_OFFSET_BAD: "
                "DW_FORM_ref1 outside of the section.");
            return DW_DLV_ERROR;
        }
        offset = *(Dwarf_Small *) attr->ar_debug_ptr;
        goto fixoffset;

    case DW_FORM_ref2:
        READ_UNALIGNED_CK(dbg, offset, Dwarf_Unsigned,
            attr->ar_debug_ptr, DWARF_HALF_SIZE,
            error,section_end);
        goto fixoffset;

    case DW_FORM_ref4:
        READ_UNALIGNED_CK(dbg, offset, Dwarf_Unsigned,
            attr->ar_debug_ptr, DWARF_32BIT_SIZE,
            error,section_end);
        goto fixoffset;
