static bfd_boolean
find_format (bfd_vma                       memaddr,
	     unsigned long long            insn,
	     unsigned int *                insn_len,
             unsigned                      isafilter,
	     struct disassemble_info *     info,
             const struct arc_opcode **    opcode_result,
             struct arc_operand_iterator * iter)
{
  const struct arc_opcode *opcode = NULL;
  // <MASK>
  arc_infop->limm_p = needs_limm;

  return TRUE;
}