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
      size_t len;
      unsigned char ext_len[4];
      char *name;
      // <MASK>
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