if (attr->ar_debug_ptr >= section_end) {
            _dwarf_error_string(dbg, error, 
                DW_DLE_ATTR_FORM_OFFSET_BAD,
                "DW_DLE_ATTR_FORM_OFFSET_BAD: "
                "DW_FORM_ref1 outside of the section.");
            return DW_DLV_ERROR;
        }
        offset = *(Dwarf_Small *) attr->ar_debug_ptr;
        goto fixoffset;