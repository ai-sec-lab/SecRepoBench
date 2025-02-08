static bfd_reloc_status_type
bpf_elf_generic_reloc (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
		       void *data, asection *input_section,
		       bfd *output_bfd ATTRIBUTE_UNUSED,
		       char **error_message ATTRIBUTE_UNUSED)
{

  bfd_signed_vma relocation;
  bfd_reloc_status_type status;
  bfd_byte *where;

  /* Sanity check that the address is in range.  */
  // <MASK>

  /*  Get the symbol value.  */
  if (bfd_is_com_section (symbol->section))
    relocation = 0;
  else
    relocation = symbol->value;

  if (symbol->flags & BSF_SECTION_SYM)
    /* Relocation against a section symbol: add in the section base address.  */
    relocation += BASEADDR (symbol->section);

  relocation += reloc_entry->addend;

  where = (bfd_byte *) data + reloc_entry->address;

  status = bfd_check_overflow (reloc_entry->howto->complain_on_overflow,
			       reloc_entry->howto->bitsize,
			       reloc_entry->howto->rightshift, 64, relocation);

  if (status != bfd_reloc_ok)
    return status;

  /* Now finally install the relocation.  */
  if (reloc_entry->howto->type == R_BPF_INSN_64)
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
      bfd_put (reloc_entry->howto->bitsize, abfd, relocation,
	       where + reloc_entry->howto->bitpos / 8);
    }

  reloc_entry->addend = relocation;
  reloc_entry->address += input_section->output_offset;

  return bfd_reloc_ok;
}