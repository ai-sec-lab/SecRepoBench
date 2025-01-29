if (!sec || !bfd_malloc_and_get_section (abfd, sec, &contents))
    return ret;
  extdyn = contents;
  extdynend = contents + sec->size;
  for (; extdyn < extdynend; extdyn += sizeof (ElfNN_External_Dyn))
    {
      Elf_Internal_Dyn dyn;
      bfd_elfNN_swap_dyn_in (abfd, extdyn, &dyn);

      /* Let's check the processor specific dynamic array tags.  */
      bfd_vma tag = dyn.d_tag;
      if (tag < DT_LOPROC || tag > DT_HIPROC)
	continue;

      switch (tag)
	{
	case DT_AARCH64_BTI_PLT:
	  ret |= PLT_BTI;
	  break;

	case DT_AARCH64_PAC_PLT:
	  ret |= PLT_PAC;
	  break;

	default: break;
	}
    }