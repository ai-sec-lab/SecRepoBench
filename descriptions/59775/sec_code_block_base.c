int length = 0;
            int reares = 0;

            if (loc_ptr >= section_end) {
                _dwarf_error_string(dbg,error,
                    DW_DLE_LOCEXPR_OFF_SECTION_END,
                    "DW_DLE_LOCEXPR_OFF_SECTION_END "
                    "at DW_OP_GNU_encoded_addr. "
                    "Corrupt DWARF");
                return DW_DLV_ERROR;
            }
            reares = read_encoded_addr(loc_ptr,dbg,
                section_end,
                address_size,
                &operand1, &length,error);