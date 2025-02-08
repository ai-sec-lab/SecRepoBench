static bfd_boolean
xcoff64_slurp_armap (bfd *abfd)
{
  file_ptr off;
  size_t namelength;
  bfd_size_type sz, amt;
  bfd_byte *contents, *cend;
  bfd_vma c, i;
  carsym *arsym;
  bfd_byte *p;
  file_ptr pos;

  /* This is for the new format.  */
  struct xcoff_ar_hdr_big hdr;

  if (xcoff_ardata (abfd) == NULL)
    {
      abfd->has_armap = FALSE;
      return TRUE;
    }

  off = bfd_scan_vma (xcoff_ardata_big (abfd)->symoff64,
		      (const char **) NULL, 10);
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
  GET_VALUE_IN_FIELD (namelength, hdr.namlen, 10);
  pos = ((namelength + 1) & ~(size_t) 1) + SXCOFFARFMAG;
  if (bfd_seek (abfd, pos, SEEK_CUR) != 0)
    return FALSE;

  sz = bfd_scan_vma (hdr.size, (const char **) NULL, 10);
  if (sz + 1 < 9)
    {
      bfd_set_error (bfd_error_bad_value);
      return FALSE;
    }

  /* Read in the entire symbol table.  */
  contents = (bfd_byte *) _bfd_alloc_and_read (abfd, sz + 1, sz);
  if (contents == NULL)
    return FALSE;

  /* Ensure strings are NULL terminated so we don't wander off the end
     of the buffer.  */
  contents[sz] = 0;

  /* The symbol table starts with an eight byte count.  */
  c = H_GET_64 (abfd, contents);

  if (c >= sz / 8)
    {
      bfd_set_error (bfd_error_bad_value);
      return FALSE;
    }
  amt = c;
  amt *= sizeof (carsym);
  bfd_ardata (abfd)->symdefs = (carsym *) bfd_alloc (abfd, amt);
  if (bfd_ardata (abfd)->symdefs == NULL)
    return FALSE;

  /* After the count comes a list of eight byte file offsets.  */
  for (i = 0, arsym = bfd_ardata (abfd)->symdefs, p = contents + 8;
       i < c;
       ++i, ++arsym, p += 8)
    arsym->file_offset = H_GET_64 (abfd, p);

  /* After the file offsets come null terminated symbol names.  */
  cend = contents + sz;
  for (i = 0, arsym = bfd_ardata (abfd)->symdefs;
       i < c;
       ++i, ++arsym, p += strlen ((char *) p) + 1)
    {
      if (p >= cend)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return FALSE;
	}
      arsym->name = (char *) p;
    }

  bfd_ardata (abfd)->symdef_count = c;
  abfd->has_armap = TRUE;

  return TRUE;
}