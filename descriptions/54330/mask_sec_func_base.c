static bfd_reloc_status_type
sh_reloc (bfd *      abfd,
	  arelent *  reloc_entry,
	  asymbol *  symbol_in,
	  void *     data,
	  asection * input_section,
	  bfd *      output_bfd,
	  char **    error_message ATTRIBUTE_UNUSED)
{
  bfd_vma insn;
  bfd_vma sym_value;
  unsigned short r_type;
  bfd_vma addr = reloc_entry->address;
  bfd_byte *hit_data = addr + (bfd_byte *) data;

  r_type = reloc_entry->howto->type;

  if (output_bfd != NULL)
    {
      /* Partial linking--do nothing.  */
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  /* Almost all relocs have to do with relaxing.  If any work must be
     done for them, it has been done in sh_relax_section.  */
  if (r_type != R_SH_IMM32
#ifdef COFF_WITH_PE
      && r_type != R_SH_IMM32CE
      && r_type != R_SH_IMAGEBASE
#endif
      && (r_type != R_SH_PCDISP
	  || (symbol_in->flags & BSF_LOCAL) != 0))
    return bfd_reloc_ok;

  if (symbol_in != NULL
      && bfd_is_und_section (symbol_in->section))
    return bfd_reloc_undefined;

  // <MASK>
}