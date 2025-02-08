static bfd_boolean
setup_sections (bfd *abfd,
		struct som_header *file_hdr,
		unsigned long current_offset)
{
  char *space_strings = NULL;
  unsigned int space_index, i;
  unsigned int total_subspaces = 0;
  asection **subspace_sections = NULL;
  asection *section;
  size_t amt;

  /* First, read in space names.  */
  amt = file_hdr->space_strings_size;
  if (amt == (size_t) -1)
    {
      bfd_set_error (bfd_error_no_memory);
      goto error_return;
    }
  if (bfd_seek (abfd, current_offset + file_hdr->space_strings_location,
		SEEK_SET) != 0)
    goto error_return;
  space_strings = (char *) _bfd_malloc_and_read (abfd, amt + 1, amt);
  if (space_strings == NULL)
    goto error_return;
  /* Make sure that the string table is NUL terminated.  */
  space_strings[amt] = 0;

  /* Loop over all of the space dictionaries, building up sections.  */
  for (space_index = 0; space_index < file_hdr->space_total; space_index++)
    {
      struct som_space_dictionary_record space;
      struct som_external_space_dictionary_record ext_space;
      char *space_name;
      struct som_external_subspace_dictionary_record ext_subspace;
      struct som_subspace_dictionary_record subspace, save_subspace;
      unsigned int subspace_index;
      asection *space_asect;
      bfd_size_type space_size = 0;
      char *newname;

      /* Read the space dictionary element.  */
      if (bfd_seek (abfd,
		    (current_offset + file_hdr->space_location
		     + space_index * sizeof (ext_space)),
		    SEEK_SET) != 0)
	goto error_return;
      amt = sizeof ext_space;
      if (bfd_bread (&ext_space, amt, abfd) != amt)
	goto error_return;

      som_swap_space_dictionary_in (&ext_space, &space);

      /* Setup the space name string.  */
      if (space.name >= file_hdr->space_strings_size)
	goto error_return;

      space_name = space.name + space_strings;

      /* Make a section out of it.  */
      amt = strlen (space_name) + 1;
      newname = bfd_alloc (abfd, amt);
      if (!newname)
	goto error_return;
      strcpy (newname, space_name);

      space_asect = bfd_make_section_anyway (abfd, newname);
      if (!space_asect)
	goto error_return;

      if (space.is_loadable == 0)
	space_asect->flags |= SEC_DEBUGGING;

      /* Set up all the attributes for the space.  */
      if (! bfd_som_set_section_attributes (space_asect, space.is_defined,
					    space.is_private, space.sort_key,
					    space.space_number))
	goto error_return;

      /* If the space has no subspaces, then we're done.  */
      if (space.subspace_quantity == 0)
	continue;

      /* Now, read in the first subspace for this space.  */
      if (bfd_seek (abfd,
		    (current_offset + file_hdr->subspace_location
		     + space.subspace_index * sizeof ext_subspace),
		    SEEK_SET) != 0)
	goto error_return;
      amt = sizeof ext_subspace;
      if (bfd_bread (&ext_subspace, amt, abfd) != amt)
	goto error_return;
      /* Seek back to the start of the subspaces for loop below.  */
      if (bfd_seek (abfd,
		    (current_offset + file_hdr->subspace_location
		     + space.subspace_index * sizeof ext_subspace),
		    SEEK_SET) != 0)
	goto error_return;

      som_swap_subspace_dictionary_in (&ext_subspace, &subspace);

      /* Setup the start address and file loc from the first subspace
	 record.  */
      space_asect->vma = subspace.subspace_start;
      space_asect->filepos = subspace.file_loc_init_value + current_offset;
      space_asect->alignment_power = exact_log2 (subspace.alignment);
      if (space_asect->alignment_power == (unsigned) -1)
	goto error_return;

      /* Initialize save_subspace so we can reliably determine if this
	 loop placed any useful values into it.  */
      memset (&save_subspace, 0, sizeof (save_subspace));

      /* Loop over the rest of the subspaces, building up more sections.  */
      for (subspace_index = 0; subspace_index < space.subspace_quantity;
	   subspace_index++)
	{
	  asection *subspace_asect;
	  char *subspace_name;

	  /* Read in the next subspace.  */
	  amt = sizeof ext_subspace;
	  if (bfd_bread (&ext_subspace, amt, abfd) != amt)
	    goto error_return;

	  som_swap_subspace_dictionary_in (&ext_subspace, &subspace);

	  /* Setup the subspace name string.  */
	  if (subspace.name >= file_hdr->space_strings_size)
	    goto error_return;

	  subspace_name = subspace.name + space_strings;

	  amt = strlen (subspace_name) + 1;
	  newname = bfd_alloc (abfd, amt);
	  if (!newname)
	    goto error_return;
	  strcpy (newname, subspace_name);

	  /* Make a section out of this subspace.  */
	  subspace_asect = bfd_make_section_anyway (abfd, newname);
	  if (!subspace_asect)
	    goto error_return;

	  /* Store private information about the section.  */
	  if (! bfd_som_set_subsection_attributes (subspace_asect, space_asect,
						   subspace.access_control_bits,
						   subspace.sort_key,
						   subspace.quadrant,
						   subspace.is_comdat,
						   subspace.is_common,
						   subspace.dup_common))
	    goto error_return;

	  /* Keep an easy mapping between subspaces and sections.
	     Note we do not necessarily read the subspaces in the
	     same order in which they appear in the object file.

	     So to make the target index come out correctly, we
	     store the location of the subspace header in target
	     index, then sort using the location of the subspace
	     header as the key.  Then we can assign correct
	     subspace indices.  */
	  total_subspaces++;
	  subspace_asect->target_index = bfd_tell (abfd) - sizeof (subspace);

	  /* Set SEC_READONLY and SEC_CODE/SEC_DATA as specified
	     by the access_control_bits in the subspace header.  */
	  switch (subspace.access_control_bits >> 4)
	    {
	    /* Readonly data.  */
	    case 0x0:
	      subspace_asect->flags |= SEC_DATA | SEC_READONLY;
	      break;

	    /* Normal data.  */
	    case 0x1:
	      subspace_asect->flags |= SEC_DATA;
	      break;

	    /* Readonly code and the gateways.
	       Gateways have other attributes which do not map
	       into anything BFD knows about.  */
	    case 0x2:
	    case 0x4:
	    case 0x5:
	    case 0x6:
	    case 0x7:
	      subspace_asect->flags |= SEC_CODE | SEC_READONLY;
	      break;

	    /* dynamic (writable) code.  */
	    case 0x3:
	      subspace_asect->flags |= SEC_CODE;
	      break;
	    }

	  if (subspace.is_comdat || subspace.is_common || subspace.dup_common)
	    subspace_asect->flags |= SEC_LINK_ONCE;

	  if (subspace.subspace_length > 0)
	    subspace_asect->flags |= SEC_HAS_CONTENTS;

	  if (subspace.is_loadable)
	    subspace_asect->flags |= SEC_ALLOC | SEC_LOAD;
	  else
	    subspace_asect->flags |= SEC_DEBUGGING;

	  if (subspace.code_only)
	    subspace_asect->flags |= SEC_CODE;

	  /* Both file_loc_init_value and initialization_length will
	     be zero for a BSS like subspace.  */
	  if (subspace.file_loc_init_value == 0
	      && subspace.initialization_length == 0)
	    subspace_asect->flags &= ~(SEC_DATA | SEC_LOAD | SEC_HAS_CONTENTS);

	  /* This subspace has relocations.
	     The fixup_request_quantity is a byte count for the number of
	     entries in the relocation stream; it is not the actual number
	     of relocations in the subspace.  */
	  if (subspace.fixup_request_quantity != 0)
	    {
	      subspace_asect->flags |= SEC_RELOC;
	      subspace_asect->rel_filepos = subspace.fixup_request_index;
	      som_section_data (subspace_asect)->reloc_size
		= subspace.fixup_request_quantity;
	      /* We can not determine this yet.  When we read in the
		 relocation table the correct value will be filled in.  */
	      subspace_asect->reloc_count = (unsigned) -1;
	    }

	  /* Update save_subspace if appropriate.  */
	  if (subspace.file_loc_init_value > save_subspace.file_loc_init_value)
	    save_subspace = subspace;

	  subspace_asect->vma = subspace.subspace_start;
	  subspace_asect->size = subspace.subspace_length;
	  subspace_asect->filepos = (subspace.file_loc_init_value
				     + current_offset);
	  subspace_asect->alignment_power = exact_log2 (subspace.alignment);
	  if (subspace_asect->alignment_power == (unsigned) -1)
	    goto error_return;

	  /* Keep track of the accumulated sizes of the sections.  */
	  space_size += subspace.subspace_length;
	}

      /* This can happen for a .o which defines symbols in otherwise
	 empty subspaces.  */
      if (!save_subspace.file_loc_init_value)
	space_asect->size = 0;
      else
	{
	  if (file_hdr->a_magic != RELOC_MAGIC)
	    {
	      /* Setup the size for the space section based upon the info
		 in the last subspace of the space.  */
	      space_asect->size = (save_subspace.subspace_start
				   - space_asect->vma
				   + save_subspace.subspace_length);
	    }
	  else
	    {
	      /* The subspace_start field is not initialised in relocatable
		 only objects, so it cannot be used for length calculations.
		 Instead we use the space_size value which we have been
		 accumulating.  This isn't an accurate estimate since it
		 ignores alignment and ordering issues.  */
	      space_asect->size = space_size;
	    }
	}
    }
  /* Now that we've read in all the subspace records, we need to assign
     a target index to each subspace.  */
  if (_bfd_mul_overflow (total_subspaces, sizeof (asection *), &amt))
    {
      bfd_set_error (bfd_error_file_too_big);
      goto error_return;
    }
  subspace_sections = bfd_malloc (amt);
  if (subspace_sections == NULL)
    goto error_return;

  for (i = 0, section = abfd->sections; section; section = section->next)
    {
      if (!som_is_subspace (section))
	continue;

      subspace_sections[i] = section;
      i++;
    }
  qsort (subspace_sections, total_subspaces,
	 sizeof (asection *), compare_subspaces);

  /* subspace_sections is now sorted in the order in which the subspaces
     appear in the object file.  Assign an index to each one now.  */
  for (i = 0; i < total_subspaces; i++)
    subspace_sections[i]->target_index = i;

  if (space_strings != NULL)
    free (space_strings);

  if (subspace_sections != NULL)
    free (subspace_sections);

  return TRUE;

 error_return:
  if (space_strings != NULL)
    free (space_strings);

  if (subspace_sections != NULL)
    free (subspace_sections);
  return FALSE;
}