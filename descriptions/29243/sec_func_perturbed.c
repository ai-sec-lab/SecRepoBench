static bfd_boolean
_bfd_vms_slurp_egsd (bfd *abfd)
{
  int gsd_type;
  unsigned int gsd_size;
  unsigned char *vms_rec;
  bfd_vma base_addr;
  long psindx;

  vms_debug2 ((2, "EGSD\n"));

  if (PRIV (recrd.rec_size) < 8)
    {
      _bfd_error_handler (_("corrupt EGSD record: its size (%#x) is too small"),
			  PRIV (recrd.rec_size));
      bfd_set_error (bfd_error_bad_value);
      return FALSE;
    }

  PRIV (recrd.rec) += 8;	/* Skip type, size, align pad.  */
  PRIV (recrd.rec_size) -= 8;

  /* Calculate base address for each section.  */
  base_addr = 0;

  while (PRIV (recrd.rec_size) > 4)
    {
      vms_rec = PRIV (recrd.rec);

      gsd_type = bfd_getl16 (vms_rec);
      gsd_size = bfd_getl16 (vms_rec + 2);

      vms_debug2 ((3, "egsd_type %d\n", gsd_type));

      /* PR 21615: Check for size overflow.  */
      if (PRIV (recrd.rec_size) < gsd_size)
	{
	  _bfd_error_handler (_("corrupt EGSD record type %d: size (%#x) "
				"is larger than remaining space (%#x)"),
			      gsd_type, gsd_size, PRIV (recrd.rec_size));
	  bfd_set_error (bfd_error_bad_value);
	  return FALSE;
	}

      if (gsd_size < 4)
	{
	too_small:
	  _bfd_error_handler (_("corrupt EGSD record type %d: size (%#x) "
				"is too small"),
			      gsd_type, gsd_size);
	  bfd_set_error (bfd_error_bad_value);
	  return FALSE;
	}

      switch (gsd_type)
	{
	case EGSD__C_PSC:
	  /* Program section definition.  */
	  {
	    struct vms_egps *egps = (struct vms_egps *) vms_rec;
	    flagword new_flags, vms_flags;
	    asection *section;

	    if (offsetof (struct vms_egps, flags) + 2 > gsd_size)
	      goto too_small;
	    vms_flags = bfd_getl16 (egps->flags);

	    if ((vms_flags & EGPS__V_REL) == 0)
	      {
		/* Use the global absolute section for all
		   absolute sections.  */
		section = bfd_abs_section_ptr;
	      }
	    else
	      {
		char *name;
		bfd_vma align_addr;
		size_t left;

		if (offsetof (struct vms_egps, namlng) >= gsd_size)
		  goto too_small;
		left = gsd_size - offsetof (struct vms_egps, namlng);
		name = _bfd_vms_save_counted_string (abfd, &egps->namlng, left);
		if (name == NULL || name[0] == 0)
		  return FALSE;

		section = bfd_make_section (abfd, name);
		if (!section)
		  return FALSE;

		section->filepos = 0;
		section->size = bfd_getl32 (egps->alloc);
		section->alignment_power = egps->align & 31;

		vms_section_data (section)->flags = vms_flags;
		vms_section_data (section)->no_flags = 0;

		new_flags = vms_secflag_by_name (evax_section_flags,
						 section->name,
						 section->size > 0);
		if (section->size > 0)
		  new_flags |= SEC_LOAD;
		if (!(vms_flags & EGPS__V_NOMOD) && section->size > 0)
		  {
		    /* Set RELOC and HAS_CONTENTS if the section is not
		       demand-zero and not empty.  */
		    new_flags |= SEC_HAS_CONTENTS;
		    if (vms_flags & EGPS__V_REL)
		      new_flags |= SEC_RELOC;
		  }
		if (vms_flags & EGPS__V_EXE)
		  {
		    /* Set CODE if section is executable.  */
		    new_flags |= SEC_CODE;
		    new_flags &= ~SEC_DATA;
		  }
		if (!bfd_set_section_flags (section, new_flags))
		  return FALSE;

		/* Give a non-overlapping vma to non absolute sections.  */
		align_addr = (bfd_vma) 1 << section->alignment_power;
		base_addr = (base_addr + align_addr - 1) & -align_addr;
		section->vma = base_addr;
		base_addr += section->size;
	      }

	    /* Append it to the section array.  */
	    if (PRIV (section_count) >= PRIV (section_max))
	      {
		if (PRIV (section_max) == 0)
		  PRIV (section_max) = 16;
		else
		  PRIV (section_max) *= 2;
		PRIV (sections) = bfd_realloc_or_free
		  (PRIV (sections), PRIV (section_max) * sizeof (asection *));
		if (PRIV (sections) == NULL)
		  return FALSE;
	      }

	    PRIV (sections)[PRIV (section_count)] = section;
	    PRIV (section_count)++;
	  }
	  break;

	case EGSD__C_SYM:
	  {
	    unsigned int nameoffset;
	    struct vms_symbol_entry *entry;
	    struct vms_egsy *egsy = (struct vms_egsy *) vms_rec;
	    flagword old_flags;

	    if (offsetof (struct vms_egsy, flags) + 2 > gsd_size)
	      goto too_small;
	    old_flags = bfd_getl16 (egsy->flags);
	    if (old_flags & EGSY__V_DEF)
	      nameoffset = ESDF__B_NAMLNG;
	    else
	      nameoffset = ESRF__B_NAMLNG;

	    if (nameoffset >= gsd_size)
	      goto too_small;
	    entry = add_symbol (abfd, vms_rec + nameoffset, gsd_size - nameoffset);
	    if (entry == NULL)
	      return FALSE;

	    /* Allow only duplicate reference.  */
	    if ((entry->flags & EGSY__V_DEF) && (old_flags & EGSY__V_DEF))
	      abort ();

	    if (entry->typ == 0)
	      {
		entry->typ = gsd_type;
		entry->data_type = egsy->datyp;
		entry->flags = old_flags;
	      }

	    if (old_flags & EGSY__V_DEF)
	      {
		struct vms_esdf *esdf = (struct vms_esdf *) vms_rec;

		entry->value = bfd_getl64 (esdf->value);
		if (PRIV (sections) == NULL)
		  return FALSE;

		psindx = bfd_getl32 (esdf->psindx);
		/* PR 21813: Check for an out of range index.  */
		if (psindx < 0 || psindx >= (int) PRIV (section_count))
		  {
		  bad_psindx:
		    _bfd_error_handler (_("corrupt EGSD record: its psindx "
					  "field is too big (%#lx)"),
					psindx);
		    bfd_set_error (bfd_error_bad_value);
		    return FALSE;
		  }
		entry->section = PRIV (sections)[psindx];

		if (old_flags & EGSY__V_NORM)
		  {
		    PRIV (norm_sym_count)++;

		    entry->code_value = bfd_getl64 (esdf->code_address);
		    psindx = bfd_getl32 (esdf->ca_psindx);
		    /* PR 21813: Check for an out of range index.  */
		    if (psindx < 0 || psindx >= (int) PRIV (section_count))
		      goto bad_psindx;
		    entry->code_section = PRIV (sections)[psindx];
		  }
	      }
	  }
	  break;

	case EGSD__C_SYMG:
	  {
	    struct vms_symbol_entry *entry;
	    struct vms_egst *egst = (struct vms_egst *)vms_rec;
	    flagword old_flags;
	    unsigned int nameoffset = offsetof (struct vms_egst, namlng);

	    if (nameoffset >= gsd_size)
	      goto too_small;
	    entry = add_symbol (abfd, &egst->namlng, gsd_size - nameoffset);
	    if (entry == NULL)
	      return FALSE;

	    old_flags = bfd_getl16 (egst->header.flags);
	    entry->typ = gsd_type;
	    entry->data_type = egst->header.datyp;
	    entry->flags = old_flags;

	    entry->symbol_vector = bfd_getl32 (egst->value);

	    if (old_flags & EGSY__V_REL)
	      {
		if (PRIV (sections) == NULL)
		  return FALSE;
		psindx = bfd_getl32 (egst->psindx);
		/* PR 21813: Check for an out of range index.  */
		if (psindx < 0 || psindx >= (int) PRIV (section_count))
		  goto bad_psindx;
		entry->section = PRIV (sections)[psindx];
	      }
	    else
	      entry->section = bfd_abs_section_ptr;

	    entry->value = bfd_getl64 (egst->lp_2);

	    if (old_flags & EGSY__V_NORM)
	      {
		PRIV (norm_sym_count)++;

		entry->code_value = bfd_getl64 (egst->lp_1);
		entry->code_section = bfd_abs_section_ptr;
	      }
	  }
	  break;

	case EGSD__C_SPSC:
	case EGSD__C_IDC:
	  /* Currently ignored.  */
	  break;
	case EGSD__C_SYMM:
	case EGSD__C_SYMV:
	default:
	  _bfd_error_handler (_("unknown EGSD subtype %d"), gsd_type);
	  bfd_set_error (bfd_error_bad_value);
	  return FALSE;
	}

      PRIV (recrd.rec_size) -= gsd_size;
      PRIV (recrd.rec) += gsd_size;
    }

  /* FIXME: Should we complain if PRIV (recrd.rec_size) is not zero ?  */

  if (PRIV (gsd_sym_count) > 0)
    abfd->flags |= HAS_SYMS;

  return TRUE;
}