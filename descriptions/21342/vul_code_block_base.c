bfd_size_type parsed_size;
  char *raw_armap;
  struct artdata *ardata;
  unsigned int count;
  char *raw_ptr;
  carsym *symdef_ptr;
  char *stringbase;
  bfd_size_type amt;

  /* Get the name of the first element.  */
  i = bfd_bread ((void *) nextname, (bfd_size_type) 16, abfd);
  if (i == 0)
      return TRUE;
  if (i != 16)
      return FALSE;

  if (bfd_seek (abfd, (file_ptr) -16, SEEK_CUR) != 0)
    return FALSE;

  /* Irix 4.0.5F apparently can use either an ECOFF armap or a
     standard COFF armap.  We could move the ECOFF armap stuff into
     bfd_slurp_armap, but that seems inappropriate since no other
     target uses this format.  Instead, we check directly for a COFF
     armap.  */
  if (CONST_STRNEQ (nextname, "/               "))
    return bfd_slurp_armap (abfd);

  /* See if the first element is an armap.  */
  if (! strneq (nextname, ecoff_backend (abfd)->armap_start, ARMAP_START_LENGTH)
      || nextname[ARMAP_HEADER_MARKER_INDEX] != ARMAP_MARKER
      || (nextname[ARMAP_HEADER_ENDIAN_INDEX] != ARMAP_BIG_ENDIAN
	  && nextname[ARMAP_HEADER_ENDIAN_INDEX] != ARMAP_LITTLE_ENDIAN)
      || nextname[ARMAP_OBJECT_MARKER_INDEX] != ARMAP_MARKER
      || (nextname[ARMAP_OBJECT_ENDIAN_INDEX] != ARMAP_BIG_ENDIAN
	  && nextname[ARMAP_OBJECT_ENDIAN_INDEX] != ARMAP_LITTLE_ENDIAN)
      || ! strneq (nextname + ARMAP_END_INDEX, ARMAP_END, sizeof ARMAP_END - 1))
    {
      abfd->has_armap = FALSE;
      return TRUE;
    }

  /* Make sure we have the right byte ordering.  */
  if (((nextname[ARMAP_HEADER_ENDIAN_INDEX] == ARMAP_BIG_ENDIAN)
       ^ (bfd_header_big_endian (abfd)))
      || ((nextname[ARMAP_OBJECT_ENDIAN_INDEX] == ARMAP_BIG_ENDIAN)
	  ^ (bfd_big_endian (abfd))))
    {
      bfd_set_error (bfd_error_wrong_format);
      return FALSE;
    }

  /* Read in the armap.  */
  ardata = bfd_ardata (abfd);
  mapdata = (struct areltdata *) _bfd_read_ar_hdr (abfd);
  if (mapdata == NULL)
    return FALSE;
  parsed_size = mapdata->parsed_size;
  free (mapdata);

  raw_armap = (char *) _bfd_alloc_and_read (abfd, parsed_size, parsed_size);
  if (raw_armap == NULL)
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_malformed_archive);
      return FALSE;
    }

  ardata->tdata = (void *) raw_armap;

  count = H_GET_32 (abfd, raw_armap);

  ardata->symdef_count = 0;
  ardata->cache = NULL;

  /* This code used to overlay the symdefs over the raw archive data,
     but that doesn't work on a 64 bit host.  */
  stringbase = raw_armap + count * 8 + 8;

#ifdef CHECK_ARMAP_HASH
  {
    unsigned int hlog;

    /* Double check that I have the hashing algorithm right by making
       sure that every symbol can be looked up successfully.  */
    hlog = 0;
    for (i = 1; i < count; i <<= 1)
      hlog++;
    BFD_ASSERT (i == count);

    raw_ptr = raw_armap + 4;
    for (i = 0; i < count; i++, raw_ptr += 8)
      {
	unsigned int name_offset, file_offset;
	unsigned int hash, rehash, srch;

	name_offset = H_GET_32 (abfd, raw_ptr);
	file_offset = H_GET_32 (abfd, (raw_ptr + 4));
	if (file_offset == 0)
	  continue;
	hash = ecoff_armap_hash (stringbase + name_offset, &rehash, count,
				 hlog);
	if (hash == i)
	  continue;

	/* See if we can rehash to this location.  */
	for (srch = (hash + rehash) & (count - 1);
	     srch != hash && srch != i;
	     srch = (srch + rehash) & (count - 1))
	  BFD_ASSERT (H_GET_32 (abfd, (raw_armap + 8 + srch * 8)) != 0);
	BFD_ASSERT (srch == i);
      }
  }

#endif /* CHECK_ARMAP_HASH */

  raw_ptr = raw_armap + 4;
  for (i = 0; i < count; i++, raw_ptr += 8)
    if (H_GET_32 (abfd, (raw_ptr + 4)) != 0)
      ++ardata->symdef_count;

  amt = ardata->symdef_count;
  amt *= sizeof (carsym);
  symdef_ptr = (carsym *) bfd_alloc (abfd, amt);
  if (!symdef_ptr)
    return FALSE;

  ardata->symdefs = symdef_ptr;

  raw_ptr = raw_armap + 4;
  for (i = 0; i < count; i++, raw_ptr += 8)
    {
      unsigned int name_offset, file_offset;

      file_offset = H_GET_32 (abfd, (raw_ptr + 4));
      if (file_offset == 0)
	continue;
      name_offset = H_GET_32 (abfd, raw_ptr);
      symdef_ptr->name = stringbase + name_offset;
      symdef_ptr->file_offset = file_offset;
      ++symdef_ptr;
    }

  ardata->first_file_filepos = bfd_tell (abfd);
  /* Pad to an even boundary.  */
  ardata->first_file_filepos += ardata->first_file_filepos % 2;

  abfd->has_armap = TRUE;

  return TRUE;