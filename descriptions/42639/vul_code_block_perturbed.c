if (intopt.size < sizeof (Elf_External_Options))
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: warning: bad `%s' option size %u smaller than"
		   " its header"),
		abfd, MIPS_ELF_OPTIONS_SECTION_NAME (abfd), intopt.size);
	      break;
	    }
	  if (ABI_64_P (abfd) && intopt.kind == ODK_REGINFO)
	    {
	      Elf64_Internal_RegInfo intreg;

	      bfd_mips_elf64_swap_reginfo_in
		(abfd,
		 ((Elf64_External_RegInfo *)
		  (l + sizeof (Elf_External_Options))),
		 &intreg);
	      elf_gp (abfd) = intreg.ri_gp_value;
	    }
	  else if (intopt.kind == ODK_REGINFO)
	    {
	      Elf32_RegInfo intreg;

	      bfd_mips_elf32_swap_reginfo_in
		(abfd,
		 ((Elf32_External_RegInfo *)
		  (l + sizeof (Elf_External_Options))),
		 &intreg);
	      elf_gp (abfd) = intreg.ri_gp_value;
	    }
	  l += intopt.size;