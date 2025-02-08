EXPORT bool
dwg_dynapi_header_set_value (Dwg_Data *restrict dwg,
                             const char *restrict fieldname,
                             const void *restrict newvalue, const bool is_utf8)
{
#ifndef HAVE_NONNULL
  if (!dwg || !fieldname || !newvalue) // cannot set NULL value
    return false;
#endif
  {
    Dwg_DYNAPI_field *f = (Dwg_DYNAPI_field *)bsearch (
        fieldname, _dwg_header_variables_fields,
        ARRAY_SIZE (_dwg_header_variables_fields) - 1, /* NULL terminated */
        sizeof (_dwg_header_variables_fields[0]), _name_struct_cmp);
    if (f)
      {
        void *old;
        // there are no malloc'd fields in the HEADER, so no need to free().
        const Dwg_Header_Variables *const _obj = &dwg->header_vars;
        // but there are several fixed-length malloced strings preR13
        static const Dwg_DYNAPI_field r11_fixed_strings[] = {
          { "MENU", "TFv", 16, OFF (struct _dwg_header_variables, MENU), 1,1,1, 1 },
          { "DIMBLK_T", "TFv", 33, OFF (struct _dwg_header_variables, DIMBLK_T), 1,1,1, 1 },
          { "DIMBLK1_T", "TFv", 33, OFF (struct _dwg_header_variables, DIMBLK1_T), 1,1,1, 1 },
          { "DIMBLK2_T", "TFv", 33, OFF (struct _dwg_header_variables, DIMBLK2_T), 1,1,1, 1 },
          { "DIMPOST", "TFv", 16, OFF (struct _dwg_header_variables, DIMPOST), 1,1,1, 1 },
          { "DIMAPOST", "TFv", 16, OFF (struct _dwg_header_variables, DIMAPOST), 1,1,1, 1 },
          { "unknown_string", "TFv", 33, OFF (struct _dwg_header_variables, unknown_string), 1,1,1, 1 },
          { "unit1_name", "TFv", 32, OFF (struct _dwg_header_variables, unit1_name), 1,1,1, 1 },
          { "unit2_name", "TFv", 32, OFF (struct _dwg_header_variables, unit2_name), 1,1,1, 1 },
          { "unit3_name", "TFv", 32, OFF (struct _dwg_header_variables, unit3_name), 1,1,1, 1 },
          { "unit4_name", "TFv", 32, OFF (struct _dwg_header_variables, unit4_name), 1,1,1, 1 },
          { 0 }
        };
        if (f->is_string && dwg->header.from_version < R_13b1)
          {
            //find_name (fieldname, r11_fixed_strings))
            for (Dwg_DYNAPI_field* f11 = (Dwg_DYNAPI_field*)&r11_fixed_strings[0]; f11->name; f11++)
              {
                if (strEQ (fieldname, f11->name))
                  {
                    f = f11;
                    break;
                  }
              }
          }
        old = &((char*)_obj)[f->offset];
        dynapi_set_helper (old, f, dwg->header.version, newvalue, is_utf8);

        // Set also FLAGS
        if (strEQc (fieldname, "CELWEIGHT"))
          {
            dwg->header_vars.FLAGS &= ~0x1f; // delete old, and set new
            dwg->header_vars.FLAGS |= dxf_revcvt_lweight (dwg->header_vars.CELWEIGHT);
          }
        else if (strEQc (fieldname, "codepage"))
          {
            dwg->header.codepage = *(BITCODE_RS*)newvalue;
          }
#define SET_HDR_FLAGS(name, bit, inverse)          \
        else if (strEQc (fieldname, #name))        \
          {                                        \
            if (dwg->header_vars.name && !inverse) \
              dwg->header_vars.FLAGS |= bit;       \
            else                                   \
              dwg->header_vars.FLAGS &= ~bit;      \
          }
        SET_HDR_FLAGS (ENDCAPS, 0x60, 0)
        SET_HDR_FLAGS (JOINSTYLE, 0x180, 0)
        SET_HDR_FLAGS (LWDISPLAY, 0x200, 1)
        SET_HDR_FLAGS (XEDIT, 0x400, 1)
        SET_HDR_FLAGS (EXTNAMES, 0x800, 0)
        SET_HDR_FLAGS (PSTYLEMODE, 0x2000, 0)
        SET_HDR_FLAGS (OLESTARTUP, 0x4000, 0)

        return true;
      }
    else
      {
        const int loglevel = dwg->opts & DWG_OPTS_LOGLEVEL;
        LOG_ERROR ("%s: Invalid header field %s", __FUNCTION__, fieldname);
        return false;
      }
  }
}