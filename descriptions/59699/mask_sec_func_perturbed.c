int
_dwarf_read_cie_fde_prefix(Dwarf_Debug dbg,
    Dwarf_Small * frame_ptr_in,
    Dwarf_Small * section_ptr_in,
    Dwarf_Unsigned section_index_in,
    Dwarf_Unsigned section_length_in,
    struct cie_fde_prefix_s *data_out,
    Dwarf_Error * error)
{
    Dwarf_Unsigned length = 0;
    int local_length_size = 0;
    int local_extension_size = 0;
    Dwarf_Small *frame_ptr = frame_ptr_in;
    Dwarf_Small *cie_ptr_addr = 0;
    Dwarf_Unsigned cieidentifier = 0;
    Dwarf_Small *section_end = section_ptr_in + section_length_in;

    // <MASK>
}