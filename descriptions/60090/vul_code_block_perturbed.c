DECODE_LEB128_UWORD_CK(frame_ptr, lreg,
            dbg,error,section_ptr_end);
        length_of_augmented_fields = (Dwarf_Unsigned) lreg;
        if (length_of_augmented_fields < dbg->de_filesize) {
            _dwarf_error_string(dbg,error,
                DW_DLE_DEBUG_FRAME_LENGTH_BAD,
                "DW_DLE_DEBUG_FRAME_LENGTH_BAD: "
                "The irix exception table length is too large "
                "to be real");
            return DW_DLV_ERROR;
        }