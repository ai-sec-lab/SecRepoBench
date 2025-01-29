Dwarf_CU_Context context = 0;
    Dwarf_Small *die_info_end = 0;

    CHECK_DIE(dwarfdie, DW_DLV_ERROR);
    context = dwarfdie->di_cu_context;