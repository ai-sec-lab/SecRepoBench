if (line_ptr > line_ptr_end) {
            _dwarf_error(dbg, err, DW_DLE_LINE_NUMBER_HEADER_ERROR);
            return DW_DLV_ERROR;
        }
        subprog_format_count = *(unsigned char *) line_ptr;
        line_ptr = line_ptr + sizeof(Dwarf_Small);
        subprog_entry_types = malloc(sizeof(Dwarf_Unsigned) *
            subprog_format_count);
        if (subprog_entry_types == NULL) {
            _dwarf_error_string(dbg, err, DW_DLE_ALLOC_FAIL,
                "DW_DLE_ALLOC_FAIL allocating subprog_entry_types");
            return DW_DLV_ERROR;
        }