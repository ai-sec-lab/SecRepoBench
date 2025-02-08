static bfd_boolean
som_bfd_fill_in_ar_symbols (bfd *abfd,
			    struct som_lst_header *lst_header,
			    carsym **syms)
{
  unsigned int i;
  carsym *set = syms[0];
  unsigned char *hash_table;
  struct som_external_som_entry *som_dict = NULL;
  bfd_size_type amt;
  file_ptr lst_filepos;
  unsigned int string_loc;

  lst_filepos = bfd_tell (abfd) - sizeof (struct som_external_lst_header);
  hash_table = bfd_malloc2 (lst_header->hash_size, 4);
  if (hash_table == NULL && lst_header->hash_size != 0)
    goto error_return;

  /* Read in the hash table.  The has table is an array of 32bit file offsets
     which point to the hash chains.  */
  amt = (bfd_size_type) lst_header->hash_size * 4;
  if (bfd_bread ((void *) hash_table, amt, abfd) != amt)
    goto error_return;

  /* Seek to and read in the SOM dictionary.  We will need this to fill
     in the carsym's filepos field.  */
  if (bfd_seek (abfd, lst_filepos + lst_header->dir_loc, SEEK_SET) != 0)
    goto error_return;

  som_dict = bfd_malloc2 (lst_header->module_count,
			  sizeof (struct som_external_som_entry));
  if (som_dict == NULL && lst_header->module_count != 0)
    goto error_return;

  amt = lst_header->module_count;
  amt *= sizeof (struct som_external_som_entry);
  if (bfd_bread ((void *) som_dict, amt, abfd) != amt)
    goto error_return;

  string_loc = lst_header->string_loc;

  /* Walk each chain filling in the carsyms as we go along.  */
  for (i = 0; i < lst_header->hash_size; i++)
    {
      struct som_external_lst_symbol_record lst_symbol;
      unsigned int hash_val;
      size_t length;
      unsigned char ext_len[4];
      char *name;
      unsigned int ndx;

      /* An empty chain has zero as it's file offset.  */
      hash_val = bfd_getb32 (hash_table + 4 * i);
      if (hash_val == 0)
	continue;

      /* Seek to and read the first symbol on the chain.  */
      if (bfd_seek (abfd, lst_filepos + hash_val, SEEK_SET) != 0)
	goto error_return;

      amt = sizeof (lst_symbol);
      if (bfd_bread ((void *) &lst_symbol, amt, abfd) != amt)
	goto error_return;

      /* Get the name of the symbol, first get the length which is stored
	 as a 32bit integer just before the symbol.

	 One might ask why we don't just read in the entire string table
	 and index into it.  Well, according to the SOM ABI the string
	 index can point *anywhere* in the archive to save space, so just
	 using the string table would not be safe.  */
      if (bfd_seek (abfd, (lst_filepos + string_loc
			   + bfd_getb32 (lst_symbol.name) - 4), SEEK_SET) != 0)
	goto error_return;

      if (bfd_bread (&ext_len, (bfd_size_type) 4, abfd) != 4)
	goto error_return;
      length = bfd_getb32 (ext_len);

      /* Allocate space for the name and null terminate it too.  */
      if (length == (size_t) -1)
	{
	  bfd_set_error (bfd_error_no_memory);
	  goto error_return;
	}
      name = bfd_zalloc (abfd, (bfd_size_type) length + 1);
      if (!name)
	goto error_return;
      if (bfd_bread (name, (bfd_size_type) length, abfd) != length)
	goto error_return;

      name[length] = 0;
      set->name = name;

      /* Fill in the file offset.  Note that the "location" field points
	 to the SOM itself, not the ar_hdr in front of it.  */
      ndx = bfd_getb32 (lst_symbol.som_index);
      if (ndx >= lst_header->module_count)
	{
	  bfd_set_error (bfd_error_bad_value);
	  goto error_return;
	}
      set->file_offset
	= bfd_getb32 (som_dict[ndx].location) - sizeof (struct ar_hdr);

      /* Go to the next symbol.  */
      set++;

      /* Iterate through the rest of the chain.  */
      while (1)
	{
	  unsigned int next_entry = bfd_getb32 (lst_symbol.next_entry);

	  if (next_entry == 0)
	    break;

	  /* Seek to the next symbol and read it in.  */
	  if (bfd_seek (abfd, lst_filepos + next_entry, SEEK_SET) != 0)
	    goto error_return;

	  amt = sizeof (lst_symbol);
	  if (bfd_bread ((void *) &lst_symbol, amt, abfd) != amt)
	    goto error_return;

	  /* Seek to the name length & string and read them in.  */
	  if (bfd_seek (abfd, lst_filepos + string_loc
			+ bfd_getb32 (lst_symbol.name) - 4, SEEK_SET) != 0)
	    goto error_return;

	  if (bfd_bread (&ext_len, (bfd_size_type) 4, abfd) != 4)
	    goto error_return;
	  length = bfd_getb32 (ext_len);

	  /* Allocate space for the name and null terminate it too.  */
	  if (length == (size_t) -1)
	    {
	      bfd_set_error (bfd_error_no_memory);
	      goto error_return;
	    }
	  name = bfd_zalloc (abfd, (bfd_size_type) length + 1);
	  if (!name)
	    goto error_return;

	  if (bfd_bread (name, (bfd_size_type) length, abfd) != length)
	    goto error_return;
	  name[length] = 0;
	  set->name = name;

	  /* Fill in the file offset.  Note that the "location" field points
	     to the SOM itself, not the ar_hdr in front of it.  */
	  ndx = bfd_getb32 (lst_symbol.som_index);
	  if (ndx >= lst_header->module_count)
	    {
	      bfd_set_error (bfd_error_bad_value);
	      goto error_return;
	    }
	  set->file_offset
	    = bfd_getb32 (som_dict[ndx].location) - sizeof (struct ar_hdr);

	  /* Go on to the next symbol.  */
	  set++;
	}
    }
  /* If we haven't died by now, then we successfully read the entire
     archive symbol table.  */
  if (hash_table != NULL)
    free (hash_table);
  if (som_dict != NULL)
    free (som_dict);
  return TRUE;

 error_return:
  if (hash_table != NULL)
    free (hash_table);
  if (som_dict != NULL)
    free (som_dict);
  return FALSE;
}