if (line_ptr > line_ptr_end) {
            _dwarf_error_string(dbg, err, DW_DLE_LINE_OFFSET_BAD,
                "DW_DLE_LINE_OFFSET_BAD "
                "The line table pointer points past end "
                "of line table.");
            return DW_DLV_ERROR;
        }
        if (actuals_table_offset > dbg->de_filesize) {
            _dwarf_error_string(dbg, err, DW_DLE_LINE_OFFSET_BAD,
                "DW_DLE_LINE_OFFSET_BAD "
                "The line table actuals offset is larger than "
                " the size of the object file. Corrupt DWARF");
            return DW_DLV_ERROR;
        }
        if ((line_ptr+actuals_table_offset) > line_ptr_end) {
            _dwarf_error_string(dbg, err, DW_DLE_LINE_OFFSET_BAD,
                "DW_DLE_LINE_OFFSET_BAD "
                "The line table actuals offset is too large "
                "to be real."); 
            return DW_DLV_ERROR;
        }