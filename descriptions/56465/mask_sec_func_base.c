int
dwarf_attrlist(Dwarf_Die die,
    Dwarf_Attribute **attrbuf,
    Dwarf_Signed     *attrcnt, Dwarf_Error *error)
{
    Dwarf_Unsigned    attr_count = 0;
    Dwarf_Unsigned    attr = 0;
    Dwarf_Unsigned    attr_form = 0;
    Dwarf_Unsigned    i = 0;
    Dwarf_Abbrev_List abbrev_list = 0;
    Dwarf_Attribute   head_attr = NULL;
    Dwarf_Attribute   curr_attr = NULL;
    Dwarf_Attribute  *last_attr = &head_attr;
    Dwarf_Debug       dbg = 0;
    Dwarf_Byte_Ptr    info_ptr = 0;
    Dwarf_Byte_Ptr    die_info_end = 0;
    int               lres = 0;
    int               bres = 0;
    Dwarf_CU_Context  context = 0;
    Dwarf_Unsigned    highest_code = 0;

    CHECK_DIE(die, DW_DLV_ERROR);
    context = die->di_cu_context;
    dbg = context->cc_dbg;
    die_info_end =
        _dwarf_calculate_info_section_end_ptr(context);
    lres = _dwarf_get_abbrev_for_code(context,
        die->di_abbrev_list->abl_code,
        &abbrev_list,
        &highest_code,error);
    if (lres == DW_DLV_ERROR) {
        return lres;
    }
    if (lres == DW_DLV_NO_ENTRY) {
        dwarfstring m;

        dwarfstring_constructor(&m);
        dwarfstring_append_printf_u(&m,
            "DW_DLE_ABBREV_MISSING "
            "There is no abbrev present for code %u "
            "in this compilation unit. ",
            die->di_abbrev_list->abl_code);
        dwarfstring_append_printf_u(&m,
            "The highest known code "
            "in any compilation unit is %u .",
            highest_code);
        _dwarf_error_string(dbg, error,
            DW_DLE_ABBREV_MISSING,
            dwarfstring_string(&m));
        dwarfstring_destructor(&m);
        return DW_DLV_ERROR;
    }

    info_ptr = die->di_debug_ptr;
    {
        /* SKIP_LEB128 */
        Dwarf_Unsigned ignore_this = 0;
        Dwarf_Unsigned len = 0;

        lres = dwarf_decode_leb128((char *)info_ptr,
            &len,&ignore_this,(char *)die_info_end);
        if (lres == DW_DLV_ERROR) {
            /* Stepped off the end SKIPping the leb  */
            dwarfstring m;

            dwarfstring_constructor(&m);
            dwarfstring_append_printf_u(&m,
                "DW_DLE_DIE_BAD: In building an attrlist "
                "we run off the end of the DIE while skipping "
                " the DIE tag, seeing the leb length as 0x%u ",
                len);
            _dwarf_error_string(dbg, error, DW_DLE_DIE_BAD,
                dwarfstring_string(&m));
            dwarfstring_destructor(&m);
            return DW_DLV_ERROR;
        }
        info_ptr += len;
    }

    // <MASK>
    if (!attr_count) {
        *attrbuf = NULL;
        *attrcnt = 0;
        return DW_DLV_NO_ENTRY;
    }
    {
        Dwarf_Attribute *attr_ptr = 0;

        attr_ptr = (Dwarf_Attribute *)
            _dwarf_get_alloc(dbg, DW_DLA_LIST, attr_count);
        if (attr_ptr == NULL) {
            empty_local_attrlist(dbg,head_attr);
            _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
            return DW_DLV_ERROR;
        }
        curr_attr = head_attr;
        for (i = 0; i < attr_count; i++) {
            *(attr_ptr + i) = curr_attr;
            curr_attr = curr_attr->ar_next;
        }
        *attrbuf = attr_ptr;
        *attrcnt = attr_count;
    }
    return DW_DLV_OK;
}