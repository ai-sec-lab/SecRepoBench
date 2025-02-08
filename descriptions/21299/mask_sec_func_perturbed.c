int
print_insn_nds32 (bfd_vma pc, disassemble_info *info)
{
  int status;
  bfd_byte buf[4];
  bfd_byte buf_data[16];
  uint64_t given;
  uint64_t given1;
  uint32_t insn;
  int symbolindex;
  int last_symbol_index = -1;
  bfd_vma addr;
  int is_data = FALSE;
  bfd_boolean found = FALSE;
  struct nds32_private_data *private_data;
  // <MASK>
}