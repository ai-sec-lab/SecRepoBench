if (fdr_ptr->isymBase < 0
	  || fdr_ptr->isymBase > symhdr->isymMax
	  || fdr_ptr->csym <= 0
	  || fdr_ptr->csym > symhdr->isymMax - fdr_ptr->isymBase
	  || fdr_ptr->issBase < 0
	  || fdr_ptr->issBase > symhdr->issMax)
	continue;
      lraw_src = ((char *) ecoff_data (abfd)->debug_info.external_sym
		  + fdr_ptr->isymBase * external_sym_size);
      lraw_end = lraw_src + fdr_ptr->csym * external_sym_size;
      for (;
	   lraw_src < lraw_end;
	   lraw_src += external_sym_size, internal_ptr++)
	{
	  SYMR internal_sym;

	  (*swap_sym_in) (abfd, (void *) lraw_src, &internal_sym);

	  if (internal_sym.iss >= symhdr->issMax - fdr_ptr->issBase
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