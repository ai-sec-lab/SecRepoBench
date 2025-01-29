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