if (section->reloc_count == (unsigned) -1)
    {
      /* Read in the external forms.  */
      if (bfd_seek (bfdfile, obj_som_reloc_filepos (bfdfile) + section->rel_filepos,
		    SEEK_SET) != 0)
	return false;
      amt = fixup_stream_size;
      external_relocs = _bfd_malloc_and_read (bfdfile, amt, amt);
      if (external_relocs == NULL)
	return false;

      /* Let callers know how many relocations found.
	 also save the relocation stream as we will
	 need it again.  */
      section->reloc_count = som_set_reloc_info (external_relocs,
						 fixup_stream_size,
						 NULL, NULL, NULL, 0, true);

      som_section_data (section)->reloc_stream = external_relocs;
    }

  /* If the caller only wanted a count, then return now.  */
  if (just_count)
    return true;

  num_relocs = section->reloc_count;
  external_relocs = som_section_data (section)->reloc_stream;
  /* Return saved information about the relocations if it is available.  */
  if (section->relocation != NULL)
    return true;

  if (_bfd_mul_overflow (num_relocs, sizeof (arelent), &amt))
    {
      bfd_set_error (bfd_error_file_too_big);
      return false;
    }
  internal_relocs = bfd_zalloc (bfdfile, amt);
  if (internal_relocs == NULL)
    return false;

  /* Process and internalize the relocations.  */
  som_set_reloc_info (external_relocs, fixup_stream_size,
		      internal_relocs, section, symbols,
		      bfd_get_symcount (bfdfile), false);