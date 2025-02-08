if (symbol_ptr->u.syment.n_numaux > ((raw_end - 1) - raw_src) / symesz)
	return NULL;

      for (i = 0;
	   i < symbol_ptr->u.syment.n_numaux;
	   i++)
	{
	  internal_ptr++;
	  raw_src += symesz;

	  bfd_coff_swap_aux_in (abfd, (void *) raw_src,
				symbol_ptr->u.syment.n_type,
				symbol_ptr->u.syment.n_sclass,
				(int) i, symbol_ptr->u.syment.n_numaux,
				&(internal_ptr->u.auxent));

	  internal_ptr->is_sym = false;
	  coff_pointerize_aux (abfd, internal, symbol_ptr, i,
			       internal_ptr, internal_end);
	}