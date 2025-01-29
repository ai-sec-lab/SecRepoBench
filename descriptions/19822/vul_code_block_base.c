
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
      len = bfd_getb32 (ext_len);

      /* Allocate space for the name and null terminate it too.  */
      if (len == (size_t) -1)
	{
	  bfd_set_error (bfd_error_no_memory);
	  goto error_return;
	}
      name = bfd_zalloc (abfd, (bfd_size_type) len + 1);
      if (!name)
	goto error_return;
      if (bfd_bread (name, (bfd_size_type) len, abfd) != len)
	goto error_return;

      name[len] = 0;
      set->name = name;

      /* Fill in the file offset.  Note that the "location" field points
	 to the SOM itself, not the ar_hdr in front of it.  */
      set->file_offset =
	bfd_getb32 (som_dict[bfd_getb32 (lst_symbol.som_index)].location)
	- sizeof (struct ar_hdr);

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
	  len = bfd_getb32 (ext_len);

	  /* Allocate space for the name and null terminate it too.  */
	  if (len == (size_t) -1)
	    {
	      bfd_set_error (bfd_error_no_memory);
	      goto error_return;
	    }
	  name = bfd_zalloc (abfd, (bfd_size_type) len + 1);
	  if (!name)
	    goto error_return;

	  if (bfd_bread (name, (bfd_size_type) len, abfd) != len)
	    goto error_return;
	  name[len] = 0;
	  set->name = name;

	  /* Fill in the file offset.  Note that the "location" field points
	     to the SOM itself, not the ar_hdr in front of it.  */
	  set->file_offset =
	    bfd_getb32 (som_dict[bfd_getb32 (lst_symbol.som_index)].location)
	    - sizeof (struct ar_hdr);

	  /* Go on to the next symbol.  */
	  set++;
	}