bfd_size_type nread;

  if (bfd_seek (abfd, where, SEEK_SET) != 0)
    return NULL;

  if (length <= sizeof (CV_INFO_PDB70) && length <= sizeof (CV_INFO_PDB20))
    return NULL;
  if (length > 256)
    length = 256;
  nread = bfd_bread (buffer, length, abfd);
  if (length != nread)
    return NULL;

  /* Ensure null termination of filename.  */
  memset (buffer + nread, 0, sizeof (buffer) - nread);