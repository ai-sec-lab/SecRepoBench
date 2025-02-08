for (; eraw_src < eraw_end; eraw_src += external_ext_size, internal_ptr++)
    {
      EXTR internal_esym;

      (*swap_ext_in) (abfd, (void *) eraw_src, &internal_esym);

      /* PR 17512: file: 3372-1000-0.004.  */
      HDRR *symhdr = &ecoff_data (abfd)->debug_info.symbolic_header;
      if (internal_esym.asym.iss >= symhdr->issExtMax
	  || internal_esym.asym.iss < 0)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      internal_ptr->symbol.name = (ecoff_data (abfd)->debug_info.ssext
				   + internal_esym.asym.iss);

      if (!ecoff_set_symbol_info (abfd, &internal_esym.asym,
				  &internal_ptr->symbol, 1,
				  internal_esym.weakext))
	return false;

      /* The alpha uses a negative ifd field for section symbols.  */
      /* PR 17512: file: 3372-1983-0.004.  */
      if (internal_esym.ifd >= symhdr->ifdMax
	  || internal_esym.ifd < 0)
	internal_ptr->fdr = NULL;
      else
	internal_ptr->fdr = (ecoff_data (abfd)->debug_info.fdr
			     + internal_esym.ifd);
      internal_ptr->local = false;
      internal_ptr->native = (void *) eraw_src;
    }

  /* The local symbols must be accessed via the fdr's, because the
     string and aux indices are relative to the fdr information.  */
  fdr_ptr = ecoff_data (abfd)->debug_info.fdr;
  fdr_end = fdr_ptr + ecoff_data (abfd)->debug_info.symbolic_header.ifdMax;
  for (; fdr_ptr < fdr_end; fdr_ptr++)
    {
      char *lraw_src;
      char *lraw_end;

      lraw_src = ((char *) ecoff_data (abfd)->debug_info.external_sym
		  + fdr_ptr->isymBase * external_sym_size);
      lraw_end = lraw_src + fdr_ptr->csym * external_sym_size;
      for (;
	   lraw_src < lraw_end;
	   lraw_src += external_sym_size, internal_ptr++)
	{
	  SYMR internal_sym;

	  (*swap_sym_in) (abfd, (void *) lraw_src, &internal_sym);

	  HDRR *symhdr = &ecoff_data (abfd)->debug_info.symbolic_header;
	  if (internal_sym.iss >= symhdr->issMax
	      || internal_sym.iss < 0)
	    {
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }
	  internal_ptr->symbol.name = (ecoff_data (abfd)->debug_info.ss
				       + fdr_ptr->issBase
				       + internal_sym.iss);
	  if (!ecoff_set_symbol_info (abfd, &internal_sym,
				      &internal_ptr->symbol, 0, 0))
	    return false;
	  internal_ptr->fdr = fdr_ptr;
	  internal_ptr->local = true;
	  internal_ptr->native = (void *) lraw_src;
	}
    }