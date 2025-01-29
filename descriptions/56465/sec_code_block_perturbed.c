if (!abbrev_list->abl_attr) {
        Dwarf_Byte_Ptr abbrev_ptr = abbrev_list->abl_abbrev_ptr;
        Dwarf_Byte_Ptr abbrev_end =
            _dwarf_calculate_abbrev_section_end_ptr(context);
        /* FIXME */
        bres = _dwarf_fill_in_attr_form_abtable(context,
            abbrev_ptr, abbrev_end, abbrev_list,
            error);
        if (bres != DW_DLV_OK) {
            return bres;
        }
    }
    /*  ASSERT  list->abl_addr and list->abl_form
        are non-null and if  list->abl_implicit_const_count > 0
        list->abl_implicit_const is non-null. */

    for ( i = 0; i <abbrev_list->abl_abbrev_count; ++i) {
        Dwarf_Signed implicit_const = 0;
        Dwarf_Half newattr_form = 0;
        int ires = 0;

        attr =  abbrev_list->abl_attr[i];
        attr_form =  abbrev_list->abl_form[i];
        if (attr > DW_AT_hi_user) {
            empty_local_attrlist(dbg,head_attr);
            _dwarf_error(dbg, error,DW_DLE_ATTR_CORRUPT);
            return DW_DLV_ERROR;
        }
        if (attr_form == DW_FORM_implicit_const) {
            implicit_const = abbrev_list->abl_implicit_const[i];
        }
        if (!_dwarf_valid_form_we_know(attr_form,attr)) {
            empty_local_attrlist(dbg,head_attr);
            _dwarf_error(dbg, error, DW_DLE_UNKNOWN_FORM);
            return DW_DLV_ERROR;
        }
        newattr_form = attr_form;
        if (attr_form == DW_FORM_indirect) {
            Dwarf_Unsigned utmp6 = 0;

            if (_dwarf_reference_outside_section(die,
                (Dwarf_Small*) info_ptr,
                ((Dwarf_Small*) info_ptr )+1)) {
                empty_local_attrlist(dbg,head_attr);
                _dwarf_error_string(dbg, error,
                    DW_DLE_ATTR_OUTSIDE_SECTION,
                    "DW_DLE_ATTR_OUTSIDE_SECTION: "
                    " Reading Attriutes: "
                    "For DW_FORM_indirect there is"
                    " no room for the form. Corrupt Dwarf");
                return DW_DLV_ERROR;
            }
            ires = _dwarf_leb128_uword_wrapper(dbg,
                &info_ptr,die_info_end,&utmp6,error);
            if (ires != DW_DLV_OK) {
                empty_local_attrlist(dbg,head_attr);
                _dwarf_error_string(dbg, error,
                    DW_DLE_ATTR_OUTSIDE_SECTION,
                    "DW_DLE_ATTR_OUTSIDE_SECTION: "
                    "Reading target of a DW_FORM_indirect "
                    "from an abbreviation failed. Corrupt Dwarf");
                return DW_DLV_ERROR;
            }
            attr_form = (Dwarf_Half) utmp6;
            if (attr_form == DW_FORM_implicit_const) {
                empty_local_attrlist(dbg,head_attr);
                _dwarf_error_string(dbg, error,
                    DW_DLE_ATTR_OUTSIDE_SECTION,
                    "DW_DLE_ATTR_OUTSIDE_SECTION: "
                    " Reading Attriutes: an indirect form "
                    "leads to a DW_FORM_implicit_const "
                    "which is not handled. Corrupt Dwarf");
                return DW_DLV_ERROR;
            }
            if (!_dwarf_valid_form_we_know(attr_form,attr)) {
                dwarfstring m;

                dwarfstring_constructor(&m);
                dwarfstring_append_printf_u(&m,
                     "DW_DLE_UNKNOWN_FORM "
                     " form indirect leads to form"
                     " of  0x%x which is unknown",
                      attr_form);
                _dwarf_error_string(dbg, error,
                    DW_DLE_UNKNOWN_FORM,
                    dwarfstring_string(&m));
                dwarfstring_destructor(&m);
                empty_local_attrlist(dbg,head_attr);
                return DW_DLV_ERROR;
            }
            newattr_form = attr_form;
        }

        if (attr) {
            Dwarf_Attribute new_attr = 0;

            new_attr = (Dwarf_Attribute)
                _dwarf_get_alloc(dbg, DW_DLA_ATTR, 1);
            if (!new_attr) {
                empty_local_attrlist(dbg,head_attr);
                _dwarf_error_string(dbg, error, DW_DLE_ALLOC_FAIL,
                    "DW_DLE_ALLOC_FAIL: attempting to allocate"
                    " a Dwarf_Attribute record");
                return DW_DLV_ERROR;
            }
            new_attr->ar_attribute = attr;
            new_attr->ar_attribute_form_direct = attr_form;
            new_attr->ar_attribute_form = newattr_form;
            /*  Here the final address must be *inside* the
                section, as we will read from there, and read
                at least one byte, we think.
                We do not want info_ptr to point past end so
                we add 1 to the end-pointer.  */
            new_attr->ar_cu_context = die->di_cu_context;
            new_attr->ar_debug_ptr = info_ptr;
            new_attr->ar_die = die;
            new_attr->ar_dbg = dbg;
            if ( attr_form != DW_FORM_implicit_const &&
                _dwarf_reference_outside_section(die,
                (Dwarf_Small*) info_ptr,
                ((Dwarf_Small*) info_ptr )+1)) {
                dwarf_dealloc_attribute(new_attr);
                empty_local_attrlist(dbg,head_attr);
                _dwarf_error_string(dbg, error,
                    DW_DLE_ATTR_OUTSIDE_SECTION,
                    "DW_DLE_ATTR_OUTSIDE_SECTION: "
                    " Reading Attriutes: "
                    "We have run off the end of the section. "
                    "Corrupt Dwarf");
                return DW_DLV_ERROR;
            }
            if (attr_form == DW_FORM_implicit_const) {
                /*  The value is here, not in a DIE.
                    Do not increment info_ptr */
                new_attr->ar_implicit_const = implicit_const;
            } else {
                Dwarf_Unsigned sov = 0;
                int vres = 0;

                vres = _dwarf_get_size_of_val(dbg,
                    attr_form,
                    die->di_cu_context->cc_version_stamp,
                    die->di_cu_context->cc_address_size,
                    info_ptr,
                    die->di_cu_context->cc_length_size,
                    &sov,
                    die_info_end,
                    error);
                if (vres!= DW_DLV_OK) {
                    dwarf_dealloc_attribute(new_attr);
                    empty_local_attrlist(dbg,head_attr);
                    return vres;
                }
                info_ptr += sov;
            }
            /*  Add to single linked list */
            *last_attr = new_attr;
            last_attr = &new_attr->ar_next;
            new_attr = 0;
            attr_count++;
        }
    }