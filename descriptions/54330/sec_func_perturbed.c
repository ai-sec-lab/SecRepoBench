static bfd_reloc_status_type
sh_reloc (bfd *      abfd,
	  arelent *  reloc_entry,
	  asymbol *  symbolinfo,
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
	  || (symbolinfo->flags & BSF_LOCAL) != 0))
    return bfd_reloc_ok;

  if (symbolinfo != NULL
      && bfd_is_und_section (symbolinfo->section))
    return bfd_reloc_undefined;

  if (!bfd_reloc_offset_in_range (reloc_entry->howto, abfd, input_section,
				  addr))
    return bfd_reloc_outofrange;

  sym_value = get_symbol_value (symbolinfo);

  switch (r_type)
    {
    case R_SH_IMM32:
#ifdef COFF_WITH_PE
    case R_SH_IMM32CE:
#endif
      insn = bfd_get_32 (abfd, hit_data);
      insn += sym_value + reloc_entry->addend;
      bfd_put_32 (abfd, insn, hit_data);
      break;
#ifdef COFF_WITH_PE
    case R_SH_IMAGEBASE:
      insn = bfd_get_32 (abfd, hit_data);
      insn += sym_value + reloc_entry->addend;
      insn -= pe_data (input_section->output_section->owner)->pe_opthdr.ImageBase;
      bfd_put_32 (abfd, insn, hit_data);
      break;
#endif
    case R_SH_PCDISP:
      insn = bfd_get_16 (abfd, hit_data);
      sym_value += reloc_entry->addend;
      sym_value -= (input_section->output_section->vma
		    + input_section->output_offset
		    + addr
		    + 4);
      sym_value += (((insn & 0xfff) ^ 0x800) - 0x800) << 1;
      insn = (insn & 0xf000) | ((sym_value >> 1) & 0xfff);
      bfd_put_16 (abfd, insn, hit_data);
      if (sym_value + 0x1000 >= 0x2000 || (sym_value & 1) != 0)
	return bfd_reloc_overflow;
      break;
    default:
      abort ();
      break;
    }

  return bfd_reloc_ok;
}