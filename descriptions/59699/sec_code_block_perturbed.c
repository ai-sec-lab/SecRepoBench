if (frame_ptr_in < section_ptr_in ||
        frame_ptr_in >= section_end) {
        _dwarf_error_string(dbg,error,DW_DLE_DEBUG_FRAME_LENGTH_BAD,
            "DW_DLE_DEBUG_FRAME_LENGTH_BAD: "
            "The frame point given _dwarf_read_cie_fde_prefix() "
            "is invalid"); 
        return DW_DLV_ERROR;
    }
    if (section_end < (frame_ptr +4)) {
        dwarfstring m;
        Dwarf_Unsigned u =
            (Dwarf_Unsigned)(uintptr_t)(frame_ptr+4) -
            (Dwarf_Unsigned)(uintptr_t)section_end;

        dwarfstring_constructor(&m);
        dwarfstring_append_printf_u(&m,
            "DW_DLE_DEBUG_FRAME_LENGTH_BAD: "
            "Reading the cie/fde prefix would "
            "put us %u bytes past the end of the "
            "frame section.  Corrupt Dwarf.",u);
        _dwarf_error_string(dbg,error,DW_DLE_DEBUG_FRAME_LENGTH_BAD,
            dwarfstring_string(&m));
        dwarfstring_destructor(&m);
        return DW_DLV_ERROR;
    }
    /* READ_AREA_LENGTH updates frame_ptr for consumed bytes */
    READ_AREA_LENGTH_CK(dbg, length, Dwarf_Unsigned,
        frame_ptr, local_length_size,
        local_extension_size,error,
        section_length_in,section_end);
    if (length == 0) {
        /*  nul bytes at end of section, seen at end of egcs eh_frame
            sections (in a.out). Take this as meaning no more CIE/FDE
            data. We should be very close to end of section. */
        return DW_DLV_NO_ENTRY;
    }
    if (length > section_length_in ||
        (length +local_length_size + local_extension_size) >
        section_length_in) {
        _dwarf_error(dbg,error,DW_DLE_DEBUG_FRAME_LENGTH_BAD);
        return DW_DLV_ERROR;
    }
    if ((frame_ptr + local_length_size) >= section_end) {
        _dwarf_error(dbg,error,DW_DLE_DEBUG_FRAME_LENGTH_BAD);
        return DW_DLV_ERROR;
    }

    cie_ptr_addr = frame_ptr;
    READ_UNALIGNED_CK(dbg, cieidentifier, Dwarf_Unsigned,
        frame_ptr, local_length_size,error,section_end);
    SIGN_EXTEND(cieidentifier, local_length_size);
    frame_ptr += local_length_size;

    data_out->cf_start_addr = frame_ptr_in;
    data_out->cf_addr_after_prefix = frame_ptr;

    data_out->cf_length = length;
    if (length > section_length_in) {
        _dwarf_error(dbg,error,DW_DLE_DEBUG_FRAME_LENGTH_BAD);
        return DW_DLV_ERROR;
    }
    if (cie_ptr_addr+length > section_end) {
        _dwarf_error(dbg,error,DW_DLE_DEBUG_FRAME_LENGTH_BAD);
        return DW_DLV_ERROR;
    }
    data_out->cf_local_length_size = local_length_size;
    data_out->cf_local_extension_size = local_extension_size;

    /*  We do not know if it is a CIE or FDE id yet.
        How we check and what it means
        depends whether it is .debug_frame
        or .eh_frame. */
    data_out->cf_cie_id = cieidentifier;

    /*  The address of the CIE_id  or FDE_id value in memory.  */
    data_out->cf_cie_id_addr = cie_ptr_addr;

    data_out->cf_section_ptr = section_ptr_in;
    data_out->cf_section_index = section_index_in;
    data_out->cf_section_length = section_length_in;
    return DW_DLV_OK;