if (relocationentry->howto->type == R_BPF_INSN_64)
    {
      bfd_size_type end = bfd_get_section_limit_octets (abfd, input_section);
      if (relocationentry->address > end
	  || end - relocationentry->address < 16)
	return bfd_reloc_outofrange;
    }
  else if (!bfd_reloc_offset_in_range (relocationentry->howto, abfd, input_section,
				       relocationentry->address))
    return bfd_reloc_outofrange;