{
      /* This is for the old format.  */
      struct xcoff_ar_hdr hdr;

      GET_VALUE_IN_FIELD (off, xcoff_ardata (abfd)->symoff, 10);
      if (off == 0)
	{
	  abfd->has_armap = FALSE;
	  return TRUE;
	}

      if (bfd_seek (abfd, off, SEEK_SET) != 0)
	return FALSE;

      /* The symbol table starts with a normal archive header.  */
      if (bfd_bread (&hdr, (bfd_size_type) SIZEOF_AR_HDR, abfd)
	  != SIZEOF_AR_HDR)
	return FALSE;

      /* Skip the name (normally empty).  */
      GET_VALUE_IN_FIELD (namlen, hdr.namlen, 10);
      off = ((namlen + 1) & ~ (size_t) 1) + SXCOFFARFMAG;
      if (bfd_seek (abfd, off, SEEK_CUR) != 0)
	return FALSE;

      GET_VALUE_IN_FIELD (sz, hdr.size, 10);
      if (sz + 1 < 5)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return FALSE;
	}

      /* Read in the entire symbol table.  */
      contents = (bfd_byte *) _bfd_alloc_and_read (abfd, sz + 1, sz);
      if (contents == NULL)
	return FALSE;

      /* Ensure strings are NULL terminated so we don't wander off the
	 end of the buffer.  */
      contents[sz] = 0;

      /* The symbol table starts with a four byte count.  */
      c = H_GET_32 (abfd, contents);

      if (c >= sz / 4)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return FALSE;
	}

      bfd_ardata (abfd)->symdefs =
	((carsym *) bfd_alloc (abfd, c * sizeof (carsym)));
      if (bfd_ardata (abfd)->symdefs == NULL)
	return FALSE;

      /* After the count comes a list of four byte file offsets.  */
      for (index = 0, arsym = bfd_ardata (abfd)->symdefs, p = contents + 4;
	   index < c;
	   ++index, ++arsym, p += 4)
	arsym->file_offset = H_GET_32 (abfd, p);
    }
  else
    {
      /* This is for the new format.  */
      struct xcoff_ar_hdr_big hdr;

      GET_VALUE_IN_FIELD (off, xcoff_ardata_big (abfd)->symoff, 10);
      if (off == 0)
	{
	  abfd->has_armap = FALSE;
	  return TRUE;
	}

      if (bfd_seek (abfd, off, SEEK_SET) != 0)
	return FALSE;

      /* The symbol table starts with a normal archive header.  */
      if (bfd_bread (&hdr, (bfd_size_type) SIZEOF_AR_HDR_BIG, abfd)
	  != SIZEOF_AR_HDR_BIG)
	return FALSE;

      /* Skip the name (normally empty).  */
      GET_VALUE_IN_FIELD (namlen, hdr.namlen, 10);
      off = ((namlen + 1) & ~ (size_t) 1) + SXCOFFARFMAG;
      if (bfd_seek (abfd, off, SEEK_CUR) != 0)
	return FALSE;

      GET_VALUE_IN_FIELD (sz, hdr.size, 10);
      if (sz + 1 < 9)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return FALSE;
	}

      /* Read in the entire symbol table.  */
      contents = (bfd_byte *) _bfd_alloc_and_read (abfd, sz + 1, sz);
      if (contents == NULL)
	return FALSE;

      /* Ensure strings are NULL terminated so we don't wander off the
	 end of the buffer.  */
      contents[sz] = 0;

      /* The symbol table starts with an eight byte count.  */
      c = H_GET_64 (abfd, contents);

      if (c >= sz / 8)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return FALSE;
	}

      bfd_ardata (abfd)->symdefs =
	((carsym *) bfd_alloc (abfd, c * sizeof (carsym)));
      if (bfd_ardata (abfd)->symdefs == NULL)
	return FALSE;

      /* After the count comes a list of eight byte file offsets.  */
      for (index = 0, arsym = bfd_ardata (abfd)->symdefs, p = contents + 8;
	   index < c;
	   ++index, ++arsym, p += 8)
	arsym->file_offset = H_GET_64 (abfd, p);
    }