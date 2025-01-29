i_phdr = elf_tdata (abfd)->phdr;
      for (i = 0; i < i_ehdrp->e_phnum; i++, i_phdr++)
	{
	  Elf_External_Phdr x_phdr;

	  if (bfd_bread (&x_phdr, sizeof x_phdr, abfd) != sizeof x_phdr)
	    goto got_no_match;
	  elf_swap_phdr_in (abfd, &x_phdr, i_phdr);
	  /* Too much code in BFD relies on alignment being a power of
	     two, as required by the ELF spec.  */
	  if (i_phdr->p_align != (i_phdr->p_align & -i_phdr->p_align))
	    {
	      i_phdr->p_align &= -i_phdr->p_align;
	      if (!abfd->read_only)
		{
		  _bfd_error_handler (_("warning: %pB has a program header "
					"with invalid alignment"), abfd);
		  abfd->read_only = 1;
		}
	    }
	  if (i_phdr->p_filesz != 0)
	    {
	      if ((i_phdr->p_offset + i_phdr->p_filesz) > filesize)
		goto got_no_match;
	      /* Try to reconstruct dynamic symbol table from PT_DYNAMIC
		 segment if there is no section header.  */
	      if (i_phdr->p_type == PT_DYNAMIC
		  && i_ehdrp->e_shstrndx == 0
		  && i_ehdrp->e_shoff == 0
		  && !_bfd_elf_get_dynamic_symbols (abfd, i_phdr,
						    elf_tdata (abfd)->phdr,
						    i_ehdrp->e_phnum,
						    filesize))
		goto got_no_match;
	    }
	}