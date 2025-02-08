int
dwarf_lowpc(Dwarf_Die die,
    Dwarf_Addr  *return_addr,
    Dwarf_Error *error)
{
    Dwarf_Addr ret_addr = 0;
    Dwarf_Byte_Ptr info_ptr = 0;
    Dwarf_Half attr_form = 0;
    Dwarf_Debug dbg = 0;
    Dwarf_Half address_size = 0;
    Dwarf_Half offset_size = 0;
    int version = 0;
    enum Dwarf_Form_Class class = DW_FORM_CLASS_UNKNOWN;
    int res = 0;
    // <MASK>
    dbg = context->cc_dbg;
    address_size = context->cc_address_size;
    offset_size = context->cc_length_size;
    res = _dwarf_get_value_ptr(die, DW_AT_low_pc,
        &attr_form,&info_ptr,0,error);
    if (res == DW_DLV_ERROR) {
        return res;
    }
    if (res == DW_DLV_NO_ENTRY) {
        return res;
    }
    version = context->cc_version_stamp;
    class = dwarf_get_form_class(version,DW_AT_low_pc,
        offset_size,attr_form);
    if (class != DW_FORM_CLASS_ADDRESS) {
        /* Not the correct form for DW_AT_low_pc */
        _dwarf_error(dbg, error, DW_DLE_LOWPC_WRONG_CLASS);
        return DW_DLV_ERROR;
    }

    if (attr_form == DW_FORM_GNU_addr_index ||
        attr_form == DW_FORM_addrx) {
        /* error is returned on dbg, not tieddbg. */
        res = _dwarf_look_in_local_and_tied(
            attr_form,
            context,
            info_ptr,
            return_addr,
            error);
        return res;
    }
    die_info_end = _dwarf_calculate_info_section_end_ptr(context);
    READ_UNALIGNED_CK(dbg, ret_addr, Dwarf_Addr,
        info_ptr, address_size,
        error,die_info_end);

    *return_addr = ret_addr;
    return DW_DLV_OK;
}