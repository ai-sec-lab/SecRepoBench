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