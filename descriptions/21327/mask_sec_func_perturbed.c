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
  // <MASK>
}