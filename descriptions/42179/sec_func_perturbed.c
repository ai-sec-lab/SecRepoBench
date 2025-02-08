static bfd_reloc_status_type
bpf_elf_generic_reloc (bfd *abfd, arelent *relocationentry, asymbol *symbol,
		       void *data, asection *input_section,
		       bfd *output_bfd ATTRIBUTE_UNUSED,
		       char **error_message ATTRIBUTE_UNUSED)
{

  bfd_signed_vma relocation;
  bfd_reloc_status_type status;
  bfd_byte *where;

  /* Sanity check that the address is in range.  */
  bfd_size_type end = bfd_get_section_limit_octets (abfd, input_section);
  bfd_size_type reloc_size;
  if (relocationentry->howto->type == R_BPF_INSN_64)
    reloc_size = 16;
  else
    reloc_size = (relocationentry->howto->bitsize
		  + relocationentry->howto->bitpos) / 8;

  if (relocationentry->address > end
      || end - relocationentry->address < reloc_size)
    return bfd_reloc_outofrange;

  /*  Get the symbol value.  */
  if (bfd_is_com_section (symbol->section))
    relocation = 0;
  else
    relocation = symbol->value;

  if (symbol->flags & BSF_SECTION_SYM)
    /* Relocation against a section symbol: add in the section base address.  */
    relocation += BASEADDR (symbol->section);

  relocation += relocationentry->addend;

  where = (bfd_byte *) data + relocationentry->address;

  status = bfd_check_overflow (relocationentry->howto->complain_on_overflow,
			       relocationentry->howto->bitsize,
			       relocationentry->howto->rightshift, 64, relocation);

  if (status != bfd_reloc_ok)
    return status;

  /* Now finally install the relocation.  */
  if (relocationentry->howto->type == R_BPF_INSN_64)
    {
      /* lddw is a 128-bit (!) instruction that allows loading a 64-bit
	 immediate into a register. the immediate is split in half, with the
	 lower 32 bits in the same position as the imm32 field of other
	 instructions, and the upper 32 bits placed at the very end of the
	 instruction. that is, there are 32 unused bits between them. */

      bfd_put_32 (abfd, (relocation & 0xFFFFFFFF), where + 4);
      bfd_put_32 (abfd, (relocation >> 32), where + 12);
    }
  else
    {
      /* For other kinds of relocations, the relocated value simply goes
	 BITPOS bits from the start of the entry. This is always a multiple
	 of 8, i.e. whole bytes.  */
      bfd_put (relocationentry->howto->bitsize, abfd, relocation,
	       where + relocationentry->howto->bitpos / 8);
    }

  relocationentry->addend = relocation;
  relocationentry->address += input_section->output_offset;

  return bfd_reloc_ok;
}