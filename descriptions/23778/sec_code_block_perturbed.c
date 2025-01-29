bfd_size_type nread;

  if (bfd_seek (abfd, where, SEEK_SET) != 0)
    return NULL;

  if (size <= sizeof (CV_INFO_PDB70) && size <= sizeof (CV_INFO_PDB20))
    return NULL;
  if (size > 256)
    size = 256;
  nread = bfd_bread (buffer, size, abfd);
  if (size != nread)
    return NULL;

  /* Ensure null termination of filename.  */
  memset (buffer + nread, 0, sizeof (buffer) - nread);