if (version == EXPERIMENTAL_LINE_TABLES_VERSION) {
        static unsigned char expbytes[5] = {0,0xff,0xff,0x7f, 0x7f };
        Dwarf_Unsigned logicals_table_offset = 0;
        Dwarf_Unsigned actuals_table_offset = 0;
        unsigned i = 0;

        for ( ; i < 5; ++i) {
            if (line_ptr >= line_ptr_end) {
                _dwarf_error(dbg, err,
                    DW_DLE_LINE_NUMBER_HEADER_ERROR);
                return DW_DLV_ERROR;
            }
            if (*line_ptr != expbytes[i]) {
                _dwarf_error(dbg, err,
                    DW_DLE_LINE_NUMBER_HEADER_ERROR);
                return DW_DLV_ERROR;
            }
            line_ptr++;
        }
        READ_UNALIGNED_CK(dbg, logicals_table_offset, Dwarf_Unsigned,
            line_ptr, local_length_size,err,line_ptr_end);
        line_context->lc_logicals_table_offset =
            logicals_table_offset;
        line_ptr += local_length_size;
        READ_UNALIGNED_CK(dbg, actuals_table_offset, Dwarf_Unsigned,
            line_ptr, local_length_size,err,line_ptr_end);
        line_context->lc_actuals_table_offset = actuals_table_offset;
        line_ptr += local_length_size;
        /* Later tests will deal with the == case as required. */
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
    }