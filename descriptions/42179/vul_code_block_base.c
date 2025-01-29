if (reloc_entry->howto->type == R_BPF_INSN_64)
    {
      bfd_size_type end = bfd_get_section_limit_octets (abfd, input_section);
      if (reloc_entry->address > end
	  || end - reloc_entry->address < 16)
	return bfd_reloc_outofrange;
    }
  else if (!bfd_reloc_offset_in_range (reloc_entry->howto, abfd, input_section,
				       reloc_entry->address))
    return bfd_reloc_outofrange;