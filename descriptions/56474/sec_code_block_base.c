Dwarf_CU_Context context = 0;
    Dwarf_Small *die_info_end = 0;

    CHECK_DIE(die, DW_DLV_ERROR);
    context = die->di_cu_context;