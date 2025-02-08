static bfd_boolean
slurp_hppa_unwind_table (Filedata *                  filedata,
			 struct hppa_unw_aux_info *  aux,
			 Elf_Internal_Shdr *         sec)
{
  unsigned long size, unw_ent_size, nentries, nrelas, i;
  Elf_Internal_Phdr * seg;
  struct hppa_unw_table_entry * tep;
  Elf_Internal_Shdr * relsec;
  Elf_Internal_Rela * rela;
  Elf_Internal_Rela * rp;
  unsigned char * table;
  unsigned char * tp;
  Elf_Internal_Sym * sym;
  const char * relname;

  /* First, find the starting address of the segment that includes
     this section.  */
  if (filedata->file_header.e_phnum)
    {
      if (! get_program_headers (filedata))
	return FALSE;

      for (seg = filedata->program_headers;
	   seg < filedata->program_headers + filedata->file_header.e_phnum;
	   ++seg)
	{
	  if (seg->p_type != PT_LOAD)
	    continue;

	  if (sec->sh_addr >= seg->p_vaddr
	      && (sec->sh_addr + sec->sh_size <= seg->p_vaddr + seg->p_memsz))
	    {
	      aux->seg_base = seg->p_vaddr;
	      break;
	    }
	}
    }

  /* Second, build the unwind table from the contents of the unwind
     section.  */
  size = sec->sh_size;
  table = (unsigned char *) get_data (NULL, filedata, sec->sh_offset, 1, size,
                                      _("unwind table"));
  if (!table)
    return FALSE;

  unw_ent_size = 16;
  nentries = size / unw_ent_size;
  size = unw_ent_size * nentries;

  // <MASK>
}