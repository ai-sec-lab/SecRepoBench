aux->table_len = nentries;
  tep = aux->table = (struct hppa_unw_table_entry *)
      xcmalloc (nentries, sizeof (aux->table[0]));

  for (tp = table; tp < table + size; tp += unw_ent_size, ++tep)
    {
      unsigned int tmp1, tmp2;

      tep->start.section = SHN_UNDEF;
      tep->end.section   = SHN_UNDEF;

      tep->start.offset = byte_get ((unsigned char *) tp + 0, 4);
      tep->end.offset = byte_get ((unsigned char *) tp + 4, 4);
      tmp1 = byte_get ((unsigned char *) tp + 8, 4);
      tmp2 = byte_get ((unsigned char *) tp + 12, 4);

      tep->start.offset += aux->seg_base;
      tep->end.offset   += aux->seg_base;

      tep->Cannot_unwind = (tmp1 >> 31) & 0x1;
      tep->Millicode = (tmp1 >> 30) & 0x1;
      tep->Millicode_save_sr0 = (tmp1 >> 29) & 0x1;
      tep->Region_description = (tmp1 >> 27) & 0x3;
      tep->reserved1 = (tmp1 >> 26) & 0x1;
      tep->Entry_SR = (tmp1 >> 25) & 0x1;
      tep->Entry_FR = (tmp1 >> 21) & 0xf;
      tep->Entry_GR = (tmp1 >> 16) & 0x1f;
      tep->Args_stored = (tmp1 >> 15) & 0x1;
      tep->Variable_Frame = (tmp1 >> 14) & 0x1;
      tep->Separate_Package_Body = (tmp1 >> 13) & 0x1;
      tep->Frame_Extension_Millicode = (tmp1 >> 12) & 0x1;
      tep->Stack_Overflow_Check = (tmp1 >> 11) & 0x1;
      tep->Two_Instruction_SP_Increment = (tmp1 >> 10) & 0x1;
      tep->Ada_Region = (tmp1 >> 9) & 0x1;
      tep->cxx_info = (tmp1 >> 8) & 0x1;
      tep->cxx_try_catch = (tmp1 >> 7) & 0x1;
      tep->sched_entry_seq = (tmp1 >> 6) & 0x1;
      tep->reserved2 = (tmp1 >> 5) & 0x1;
      tep->Save_SP = (tmp1 >> 4) & 0x1;
      tep->Save_RP = (tmp1 >> 3) & 0x1;
      tep->Save_MRP_in_frame = (tmp1 >> 2) & 0x1;
      tep->extn_ptr_defined = (tmp1 >> 1) & 0x1;
      tep->Cleanup_defined = tmp1 & 0x1;

      tep->MPE_XL_interrupt_marker = (tmp2 >> 31) & 0x1;
      tep->HP_UX_interrupt_marker = (tmp2 >> 30) & 0x1;
      tep->Large_frame = (tmp2 >> 29) & 0x1;
      tep->Pseudo_SP_Set = (tmp2 >> 28) & 0x1;
      tep->reserved4 = (tmp2 >> 27) & 0x1;
      tep->Total_frame_size = tmp2 & 0x7ffffff;
    }
  free (table);

  /* Third, apply any relocations to the unwind table.  */
  for (relsec = filedata->section_headers;
       relsec < filedata->section_headers + filedata->file_header.e_shnum;
       ++relsec)
    {
      if (relsec->sh_type != SHT_RELA
	  || relsec->sh_info >= filedata->file_header.e_shnum
	  || filedata->section_headers + relsec->sh_info != sec)
	continue;

      if (!slurp_rela_relocs (filedata, relsec->sh_offset, relsec->sh_size,
			      & rela, & nrelas))
	return FALSE;

      for (rp = rela; rp < rela + nrelas; ++rp)
	{
	  unsigned int sym_ndx;
	  unsigned int r_type = get_reloc_type (filedata, rp->r_info);
	  relname = elf_hppa_reloc_type (r_type);

	  if (relname == NULL)
	    {
	      warn (_("Skipping unknown relocation type: %u\n"), r_type);
	      continue;
	    }

	  /* R_PARISC_SEGREL32 or R_PARISC_SEGREL64.  */
	  if (! const_strneq (relname, "R_PARISC_SEGREL"))
	    {
	      warn (_("Skipping unexpected relocation type: %s\n"), relname);
	      continue;
	    }

	  i = rp->r_offset / unw_ent_size;
	  if (i >= aux->table_len)
	    {
	      warn (_("Skipping reloc with overlarge offset: %lx\n"), i);
	      continue;
	    }

	  sym_ndx = get_reloc_symindex (rp->r_info);
	  if (sym_ndx >= aux->nsyms)
	    {
	      warn (_("Skipping reloc with invalid symbol index: %u\n"),
		    sym_ndx);
	      continue;
	    }
	  sym = aux->symtab + sym_ndx;

	  switch ((rp->r_offset % unw_ent_size) / 4)
	    {
	    case 0:
	      aux->table[i].start.section = sym->st_shndx;
	      aux->table[i].start.offset  = sym->st_value + rp->r_addend;
	      break;
	    case 1:
	      aux->table[i].end.section   = sym->st_shndx;
	      aux->table[i].end.offset    = sym->st_value + rp->r_addend;
	      break;
	    default:
	      break;
	    }
	}

      free (rela);
    }

  return TRUE;