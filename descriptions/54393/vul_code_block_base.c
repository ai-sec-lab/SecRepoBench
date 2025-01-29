const Dwg_Header_Variables *const _obj = &dwg->header_vars;
        // but there are several fixed-length malloced strings preR13
        static const Dwg_DYNAPI_field r11_fixed_strings[] = {
          { "MENU", "TFv", 15, OFF (struct _dwg_header_variables, MENU), 1,1,1, 1 },
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