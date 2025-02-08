combined_entry_type *
coff_get_normalized_symtab (bfd *bfdptr)
{
  combined_entry_type *internal;
  combined_entry_type *internal_ptr;
  combined_entry_type *symbol_ptr;
  combined_entry_type *internal_end;
  size_t symesz;
  char *raw_src;
  char *raw_end;
  const char *string_table = NULL;
  asection * debug_sec = NULL;
  char *debug_sec_data = NULL;
  bfd_size_type size;

  if (obj_raw_syments (bfdptr) != NULL)
    return obj_raw_syments (bfdptr);

  if (! _bfd_coff_get_external_symbols (bfdptr))
    return NULL;

  size = obj_raw_syment_count (bfdptr);
  /* Check for integer overflow.  */
  if (size > (bfd_size_type) -1 / sizeof (combined_entry_type))
    return NULL;
  size *= sizeof (combined_entry_type);
  internal = (combined_entry_type *) bfd_zalloc (bfdptr, size);
  if (internal == NULL && size != 0)
    return NULL;
  internal_end = internal + obj_raw_syment_count (bfdptr);

  raw_src = (char *) obj_coff_external_syms (bfdptr);

  /* Mark the end of the symbols.  */
  symesz = bfd_coff_symesz (bfdptr);
  raw_end = PTR_ADD (raw_src, obj_raw_syment_count (bfdptr) * symesz);

  /* FIXME SOMEDAY.  A string table size of zero is very weird, but
     probably possible.  If one shows up, it will probably kill us.  */

  /* Swap all the raw entries.  */
  for (internal_ptr = internal;
       raw_src < raw_end;
       raw_src += symesz, internal_ptr++)
    {
      unsigned int i;

      bfd_coff_swap_sym_in (bfdptr, (void *) raw_src,
			    (void *) & internal_ptr->u.syment);
      symbol_ptr = internal_ptr;
      internal_ptr->is_sym = true;

      /* PR 17512: Prevent buffer overrun.  */
      if (symbol_ptr->u.syment.n_numaux > ((raw_end - 1) - raw_src) / symesz)
	return NULL;

      for (i = 0;
	   i < symbol_ptr->u.syment.n_numaux;
	   i++)
	{
	  internal_ptr++;
	  raw_src += symesz;

	  bfd_coff_swap_aux_in (bfdptr, (void *) raw_src,
				symbol_ptr->u.syment.n_type,
				symbol_ptr->u.syment.n_sclass,
				(int) i, symbol_ptr->u.syment.n_numaux,
				&(internal_ptr->u.auxent));

	  internal_ptr->is_sym = false;
	  coff_pointerize_aux (bfdptr, internal, symbol_ptr, i,
			       internal_ptr, internal_end);
	}
    }

  /* Free the raw symbols.  */
  if (obj_coff_external_syms (bfdptr) != NULL
      && ! obj_coff_keep_syms (bfdptr))
    {
      free (obj_coff_external_syms (bfdptr));
      obj_coff_external_syms (bfdptr) = NULL;
    }

  for (internal_ptr = internal; internal_ptr < internal_end;
       internal_ptr++)
    {
      BFD_ASSERT (internal_ptr->is_sym);

      if (internal_ptr->u.syment.n_sclass == C_FILE
	  && internal_ptr->u.syment.n_numaux > 0)
	{
	  combined_entry_type * aux = internal_ptr + 1;

	  /* Make a file symbol point to the name in the auxent, since
	     the text ".file" is redundant.  */
	  BFD_ASSERT (! aux->is_sym);

	  if (aux->u.auxent.x_file.x_n.x_n.x_zeroes == 0)
	    {
	      /* The filename is a long one, point into the string table.  */
	      if (string_table == NULL)
		{
		  string_table = _bfd_coff_read_string_table (bfdptr);
		  if (string_table == NULL)
		    return NULL;
		}

	      if ((bfd_size_type)(aux->u.auxent.x_file.x_n.x_n.x_offset)
		  >= obj_coff_strings_len (bfdptr))
		internal_ptr->u.syment._n._n_n._n_offset =
		  (uintptr_t) _("<corrupt>");
	      else
		internal_ptr->u.syment._n._n_n._n_offset =
		  (uintptr_t) (string_table
			       + aux->u.auxent.x_file.x_n.x_n.x_offset);
	    }
	  else
	    {
	      /* Ordinary short filename, put into memory anyway.  The
		 Microsoft PE tools sometimes store a filename in
		 multiple AUX entries.  */
	      if (internal_ptr->u.syment.n_numaux > 1 && obj_pe (bfdptr))
		internal_ptr->u.syment._n._n_n._n_offset =
		  ((uintptr_t)
		   copy_name (bfdptr,
			      aux->u.auxent.x_file.x_n.x_fname,
			      internal_ptr->u.syment.n_numaux * symesz));
	      else
		internal_ptr->u.syment._n._n_n._n_offset =
		  ((uintptr_t)
		   copy_name (bfdptr,
			      aux->u.auxent.x_file.x_n.x_fname,
			      (size_t) bfd_coff_filnmlen (bfdptr)));
	    }

	  /* Normalize other strings available in C_FILE aux entries.  */
	  if (!obj_pe (bfdptr))
	    for (int numaux = 1; numaux < internal_ptr->u.syment.n_numaux; numaux++)
	      {
		aux = internal_ptr + numaux + 1;
		BFD_ASSERT (! aux->is_sym);

		if (aux->u.auxent.x_file.x_n.x_n.x_zeroes == 0)
		  {
		    /* The string information is a long one, point into the string table.  */
		    if (string_table == NULL)
		      {
			string_table = _bfd_coff_read_string_table (bfdptr);
			if (string_table == NULL)
			  return NULL;
		      }

		    if ((bfd_size_type)(aux->u.auxent.x_file.x_n.x_n.x_offset)
			>= obj_coff_strings_len (bfdptr))
		      aux->u.auxent.x_file.x_n.x_n.x_offset =
			(uintptr_t) _("<corrupt>");
		    else
		      aux->u.auxent.x_file.x_n.x_n.x_offset =
			(uintptr_t) (string_table
				     + (aux->u.auxent.x_file.x_n.x_n.x_offset));
		  }
		else
		  aux->u.auxent.x_file.x_n.x_n.x_offset =
		    ((uintptr_t)
		     copy_name (bfdptr,
				aux->u.auxent.x_file.x_n.x_fname,
				(size_t) bfd_coff_filnmlen (bfdptr)));
	      }

	}
      else
	{
	  if (internal_ptr->u.syment._n._n_n._n_zeroes != 0)
	    {
	      /* This is a "short" name.  Make it long.  */
	      size_t i;
	      char *newstring;

	      /* Find the length of this string without walking into memory
		 that isn't ours.  */
	      for (i = 0; i < 8; ++i)
		if (internal_ptr->u.syment._n._n_name[i] == '\0')
		  break;

	      newstring = (char *) bfd_zalloc (bfdptr, (bfd_size_type) (i + 1));
	      if (newstring == NULL)
		return NULL;
	      strncpy (newstring, internal_ptr->u.syment._n._n_name, i);
	      internal_ptr->u.syment._n._n_n._n_offset = (uintptr_t) newstring;
	      internal_ptr->u.syment._n._n_n._n_zeroes = 0;
	    }
	  else if (internal_ptr->u.syment._n._n_n._n_offset == 0)
	    internal_ptr->u.syment._n._n_n._n_offset = (uintptr_t) "";
	  else if (!bfd_coff_symname_in_debug (bfdptr, &internal_ptr->u.syment))
	    {
	      /* Long name already.  Point symbol at the string in the
		 table.  */
	      if (string_table == NULL)
		{
		  string_table = _bfd_coff_read_string_table (bfdptr);
		  if (string_table == NULL)
		    return NULL;
		}
	      if (internal_ptr->u.syment._n._n_n._n_offset >= obj_coff_strings_len (bfdptr)
		  || string_table + internal_ptr->u.syment._n._n_n._n_offset < string_table)
		internal_ptr->u.syment._n._n_n._n_offset =
		  (uintptr_t) _("<corrupt>");
	      else
		internal_ptr->u.syment._n._n_n._n_offset =
		  ((uintptr_t) (string_table
				+ internal_ptr->u.syment._n._n_n._n_offset));
	    }
	  else
	    {
	      /* Long name in debug section.  Very similar.  */
	      if (debug_sec_data == NULL)
		debug_sec_data = build_debug_section (bfdptr, & debug_sec);
	      if (debug_sec_data != NULL)
		{
		  BFD_ASSERT (debug_sec != NULL);
		  /* PR binutils/17512: Catch out of range offsets into the debug data.  */
		  if (internal_ptr->u.syment._n._n_n._n_offset > debug_sec->size
		      || debug_sec_data + internal_ptr->u.syment._n._n_n._n_offset < debug_sec_data)
		    internal_ptr->u.syment._n._n_n._n_offset =
		      (uintptr_t) _("<corrupt>");
		  else
		    internal_ptr->u.syment._n._n_n._n_offset =
		      (uintptr_t) (debug_sec_data
				   + internal_ptr->u.syment._n._n_n._n_offset);
		}
	      else
		internal_ptr->u.syment._n._n_n._n_offset = (uintptr_t) "";
	    }
	}
      internal_ptr += internal_ptr->u.syment.n_numaux;
    }

  obj_raw_syments (bfdptr) = internal;
  BFD_ASSERT (obj_raw_syment_count (bfdptr)
	      == (unsigned int) (internal_ptr - internal));

  return internal;
}