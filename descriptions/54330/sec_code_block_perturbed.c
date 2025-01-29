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