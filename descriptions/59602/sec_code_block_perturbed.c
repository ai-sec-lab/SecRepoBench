case DW_OP_GNU_const_type:       /* 0xf4 */
        {
        /* die offset as uleb. cu-relative */
        DECODE_LEB128_UWORD_LEN_CK(loc_ptr, operand1,leb128_length,
            dbg,error_ptr,section_end);
        offset = offset + leb128_length;

        if (loc_ptr >= section_end) {
            _dwarf_error_string(dbg,error_ptr,
                DW_DLE_LOCEXPR_OFF_SECTION_END,
                "DW_DLE_LOCEXPR_OFF_SECTION_END: Error reading "
                "DW_OP_const_type/DW_OP_GNU_const_type content");
            return DW_DLV_ERROR;
        }
        /*  Next byte is size of following data block.  */
        operand2 = *loc_ptr;
        loc_ptr = loc_ptr + 1;
        offset = offset + 1;

        /*  Operand 3 points to a value in the block of size
            just gotten as operand2.
            It must fit in a Dwarf_Unsigned.
            Get the type from the die at operand1
            (a CU relative offset). */
        /*  FIXME: We should do something very different than
            what we do here! */
        operand3 = (Dwarf_Unsigned)(uintptr_t)loc_ptr;
        loc_ptr = loc_ptr + operand2;
        if (loc_ptr > section_end) {
            _dwarf_error(dbg,error_ptr,DW_DLE_LOCEXPR_OFF_SECTION_END);
            return DW_DLV_ERROR;
        }
        offset = offset + operand2;
        }
        break;

    case DW_OP_regval_type:           /* DWARF5 */
    case DW_OP_GNU_regval_type:       /* 0xf5 */
        /* reg num uleb*/
        DECODE_LEB128_UWORD_LEN_CK(loc_ptr, operand1,leb128_length,
            dbg,error_ptr,section_end);
        offset = offset + leb128_length;
        /* cu die off uleb*/
        DECODE_LEB128_UWORD_LEN_CK(loc_ptr, operand2,leb128_length,
            dbg,error_ptr,section_end);
        offset = offset + leb128_length;
        break;
    case DW_OP_convert:           /* DWARF5 */
    case DW_OP_GNU_convert:       /* 0xf7 */
    case DW_OP_reinterpret:       /* DWARF5 */
    case DW_OP_GNU_reinterpret:       /* 0xf9 */
        /* die offset  or zero */
        DECODE_LEB128_UWORD_LEN_CK(loc_ptr, operand1,leb128_length,
            dbg,error_ptr,section_end);
        offset = offset + leb128_length;
        break;
    case DW_OP_GNU_parameter_ref :       /* 0xfa */
        /* 4 byte unsigned int */
        READ_UNALIGNED_CK(dbg, operand1, Dwarf_Unsigned, loc_ptr, 4,
            error_ptr,section_end);;
        loc_ptr = loc_ptr + 4;
        if (loc_ptr > section_end) {
            _dwarf_error_string(dbg,error_ptr,
                DW_DLE_LOCEXPR_OFF_SECTION_END,
                "DW_DLE_LOCEXPR_OFF_SECTION_END: Error reading "
                "DW_OP_GNU_parameter_ref.");
            return DW_DLV_ERROR;
        }
        offset = offset + 4;
        break;