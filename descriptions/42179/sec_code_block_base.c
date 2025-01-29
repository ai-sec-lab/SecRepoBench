bfd_size_type end = bfd_get_section_limit_octets (abfd, input_section);
  bfd_size_type reloc_size;
  if (reloc_entry->howto->type == R_BPF_INSN_64)
    reloc_size = 16;
  else
    reloc_size = (reloc_entry->howto->bitsize
		  + reloc_entry->howto->bitpos) / 8;

  if (reloc_entry->address > end
      || end - reloc_entry->address < reloc_size)
    return bfd_reloc_outofrange;