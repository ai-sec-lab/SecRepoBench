if (line_ptr > line_ptr_end) {
            _dwarf_error(dbg, err, DW_DLE_LINE_OFFSET_BAD);
            return DW_DLV_ERROR;
        }