int
print_insn_metag (bfd_vma pc, disassemble_info *outf)
{
  bfd_byte buf[4];
  unsigned int insn_word;
  size_t i;
  // <MASK>
  insn_word = bfd_getl32 (buf);

  for (i = 0; i < sizeof(metag_optab)/sizeof(metag_optab[0]); i++)
    {
      const insn_template *template = &metag_optab[i];

      if ((insn_word & template->meta_mask) == template->meta_opcode)
	{
	  enum insn_encoding encoding = template->encoding;
	  insn_printer printer = insn_printers[encoding];

	  if (printer)
	    printer (insn_word, pc, template, outf);

	  return 4;
	}
    }

  return 4;
}