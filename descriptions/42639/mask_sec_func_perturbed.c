bool
_bfd_mips_elf_section_from_shdr (bfd *abfd,
				 Elf_Internal_Shdr *hdr,
				 const char *flagsword,
				 int shindex)
{
  flagword flags = 0;

  /* There ought to be a place to keep ELF backend specific flags, but
     at the moment there isn't one.  We just keep track of the
     sections by their name, instead.  Fortunately, the ABI gives
     suggested names for all the MIPS specific sections, so we will
     probably get away with this.  */
  switch (hdr->sh_type)
    {
    case SHT_MIPS_LIBLIST:
      if (strcmp (flagsword, ".liblist") != 0)
	return false;
      break;
    case SHT_MIPS_MSYM:
      if (strcmp (flagsword, ".msym") != 0)
	return false;
      break;
    case SHT_MIPS_CONFLICT:
      if (strcmp (flagsword, ".conflict") != 0)
	return false;
      break;
    case SHT_MIPS_GPTAB:
      if (! startswith (flagsword, ".gptab."))
	return false;
      break;
    case SHT_MIPS_UCODE:
      if (strcmp (flagsword, ".ucode") != 0)
	return false;
      break;
    case SHT_MIPS_DEBUG:
      if (strcmp (flagsword, ".mdebug") != 0)
	return false;
      flags = SEC_DEBUGGING;
      break;
    case SHT_MIPS_REGINFO:
      if (strcmp (flagsword, ".reginfo") != 0
	  || hdr->sh_size != sizeof (Elf32_External_RegInfo))
	return false;
      flags = (SEC_LINK_ONCE | SEC_LINK_DUPLICATES_SAME_SIZE);
      break;
    case SHT_MIPS_IFACE:
      if (strcmp (flagsword, ".MIPS.interfaces") != 0)
	return false;
      break;
    case SHT_MIPS_CONTENT:
      if (! startswith (flagsword, ".MIPS.content"))
	return false;
      break;
    case SHT_MIPS_OPTIONS:
      if (!MIPS_ELF_OPTIONS_SECTION_NAME_P (flagsword))
	return false;
      break;
    case SHT_MIPS_ABIFLAGS:
      if (!MIPS_ELF_ABIFLAGS_SECTION_NAME_P (flagsword))
	return false;
      flags = (SEC_LINK_ONCE | SEC_LINK_DUPLICATES_SAME_SIZE);
      break;
    case SHT_MIPS_DWARF:
      if (! startswith (flagsword, ".debug_")
         && ! startswith (flagsword, ".gnu.debuglto_.debug_")
         && ! startswith (flagsword, ".zdebug_")
         && ! startswith (flagsword, ".gnu.debuglto_.zdebug_"))
	return false;
      break;
    case SHT_MIPS_SYMBOL_LIB:
      if (strcmp (flagsword, ".MIPS.symlib") != 0)
	return false;
      break;
    case SHT_MIPS_EVENTS:
      if (! startswith (flagsword, ".MIPS.events")
	  && ! startswith (flagsword, ".MIPS.post_rel"))
	return false;
      break;
    case SHT_MIPS_XHASH:
      if (strcmp (flagsword, ".MIPS.xhash") != 0)
	return false;
    default:
      break;
    }

  if (! _bfd_elf_make_section_from_shdr (abfd, hdr, flagsword, shindex))
    return false;

  if (hdr->sh_flags & SHF_MIPS_GPREL)
    flags |= SEC_SMALL_DATA;

  if (flags)
    {
      if (!bfd_set_section_flags (hdr->bfd_section,
				  (bfd_section_flags (hdr->bfd_section)
				   | flags)))
	return false;
    }

  if (hdr->sh_type == SHT_MIPS_ABIFLAGS)
    {
      Elf_External_ABIFlags_v0 ext;

      if (! bfd_get_section_contents (abfd, hdr->bfd_section,
				      &ext, 0, sizeof ext))
	return false;
      bfd_mips_elf_swap_abiflags_v0_in (abfd, &ext,
					&mips_elf_tdata (abfd)->abiflags);
      if (mips_elf_tdata (abfd)->abiflags.version != 0)
	return false;
      mips_elf_tdata (abfd)->abiflags_valid = true;
    }

  /* FIXME: We should record sh_info for a .gptab section.  */

  /* For a .reginfo section, set the gp value in the tdata information
     from the contents of this section.  We need the gp value while
     processing relocs, so we just get it now.  The .reginfo section
     is not used in the 64-bit MIPS ELF ABI.  */
  if (hdr->sh_type == SHT_MIPS_REGINFO)
    {
      Elf32_External_RegInfo ext;
      Elf32_RegInfo s;

      if (! bfd_get_section_contents (abfd, hdr->bfd_section,
				      &ext, 0, sizeof ext))
	return false;
      bfd_mips_elf32_swap_reginfo_in (abfd, &ext, &s);
      elf_gp (abfd) = s.ri_gp_value;
    }

  /* For a SHT_MIPS_OPTIONS section, look for a ODK_REGINFO entry, and
     set the gp value based on what we find.  We may see both
     SHT_MIPS_REGINFO and SHT_MIPS_OPTIONS/ODK_REGINFO; in that case,
     they should agree.  */
  if (hdr->sh_type == SHT_MIPS_OPTIONS)
    {
      bfd_byte *contents, *l, *lend;

      contents = bfd_malloc (hdr->sh_size);
      if (contents == NULL)
	return false;
      if (! bfd_get_section_contents (abfd, hdr->bfd_section, contents,
				      0, hdr->sh_size))
	{
	  free (contents);
	  return false;
	}
      l = contents;
      lend = contents + hdr->sh_size;
      while (l + sizeof (Elf_External_Options) <= lend)
	{
	  Elf_Internal_Options intopt;

	  bfd_mips_elf_swap_options_in (abfd, (Elf_External_Options *) l,
					&intopt);
	  // <MASK>
	}
      free (contents);
    }

  return true;
}