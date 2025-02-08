static void
print_dynamic_symbol (Filedata *filedata, unsigned long si,
		      Elf_Internal_Sym *symtab,
		      Elf_Internal_Shdr *section,
		      char *strtab, size_t strtab_size)
{
  const char *version_string;
  enum versioned_symbol_info sym_info;
  unsigned short vna_other;
  Elf_Internal_Sym *psym = symtab + si;
  
  printf ("%6ld: ", si);
  print_vma (psym->st_value, LONG_HEX);
  putchar (' ');
  print_vma (psym->st_size, DEC_5);
  printf (" %-7s", get_symbol_type (filedata, ELF_ST_TYPE (psym->st_info)));
  printf (" %-6s", get_symbol_binding (filedata, ELF_ST_BIND (psym->st_info)));
  if (filedata->file_header.e_ident[EI_OSABI] == ELFOSABI_SOLARIS)
    printf (" %-7s",  get_solaris_symbol_visibility (psym->st_other));
  else
    {
      unsigned int vis = ELF_ST_VISIBILITY (psym->st_other);

      printf (" %-7s", get_symbol_visibility (vis));
      /* Check to see if any other bits in the st_other field are set.
	 Note - displaying this information disrupts the layout of the
	 table being generated, but for the moment this case is very rare.  */
      if (psym->st_other ^ vis)
	printf (" [%s] ", get_symbol_other (filedata, psym->st_other ^ vis));
    }
  printf (" %4s ", get_symbol_index_type (filedata, psym->st_shndx));

  bfd_boolean is_valid = VALID_SYMBOL_NAME (strtab, strtab_size,
					    psym->st_name);
  const char * sstr = is_valid  ? strtab + psym->st_name : _("<corrupt>");

  version_string
    = get_symbol_version_string (filedata,
				 (section == NULL
				  || section->sh_type == SHT_DYNSYM),
				 strtab, strtab_size, si,
				 psym, &sym_info, &vna_other);
  
  int len_avail = 21;
  if (! do_wide && version_string != NULL)
    {
      // <MASK>
    }

  print_symbol (len_avail, sstr);
    
  if (version_string)
    {
      if (sym_info == symbol_undefined)
	printf ("@%s (%d)", version_string, vna_other);
      else
	printf (sym_info == symbol_hidden ? "@%s" : "@@%s",
		version_string);
    }

  putchar ('\n');

  if (ELF_ST_BIND (psym->st_info) == STB_LOCAL
      && section != NULL
      && si >= section->sh_info
      /* Irix 5 and 6 MIPS binaries are known to ignore this requirement.  */
      && filedata->file_header.e_machine != EM_MIPS
      /* Solaris binaries have been found to violate this requirement as
	 well.  Not sure if this is a bug or an ABI requirement.  */
      && filedata->file_header.e_ident[EI_OSABI] != ELFOSABI_SOLARIS)
    warn (_("local symbol %lu found at index >= %s's sh_info value of %u\n"),
	  si, printable_section_name (filedata, section), section->sh_info);
}